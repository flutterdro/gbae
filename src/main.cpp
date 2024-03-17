
#include <cstddef>
#include <exception>
#include <filesystem>
#include <functional>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ranges>
#include <expected>
#include <span>
#include <coroutine>

#include <fmt/core.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <utility>
#include "cpu/instruction-implementation.hpp"
#include "cpu/registermanager.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "backend/shader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "utility/fatexception.hpp"
#include "emulator/gbaemu.hpp"
#include "emulator/cpu/disassembler/disassembler.hpp"

constexpr GLuint width = 1280;
constexpr GLuint height = 720;

using window_ptr = std::unique_ptr<GLFWwindow, decltype([](auto& a) {glfwDestroyWindow(a);})>;

auto test1() -> std::expected<double, std::error_code> {
    return 0;
}
#define TRY(result) result.has_value() ? *result  
auto test2() -> std::expected<double, std::error_code> {
    auto res = test1();
}

[[nodiscard]]auto create_window() 
    -> window_ptr;
auto create_projection()
    -> void;

static void pre_call_gl_callback(const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLAD_UNUSED(len_args);

    if (apiproc == NULL) {
        spdlog::error("glad: {} is NULL!", name);
        return;
    }
    if (glad_glGetError == NULL) {
        spdlog::error("glad: glGetError is NULL!");
        return;
    }

    (void) glad_glGetError();
}
static void post_call_gl_callback(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLenum error_code;

    GLAD_UNUSED(ret);
    GLAD_UNUSED(apiproc);
    GLAD_UNUSED(len_args);

    error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR) {
        spdlog::error("glad: {} in {}!", error_code, name);
    }
}

namespace ImGui {
template <typename... Ts>
IMGUI_API void TextFmt(fmt::format_string<Ts...> fmt, Ts&&... args) {
    std::string str = fmt::format(fmt, std::forward<Ts>(args)...);
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}
template <typename... Ts>
IMGUI_API void vTextFmt(fmt::string_view fmt, Ts&&... args) {
    std::string str = fmt::vformat(fmt, fmt::make_format_args(args...));
    ImGui::TextUnformatted(&*str.begin(), &*str.end());
}
}


void display_psr(fgba::cpu::psr psr) {
    ImGui::BeginGroup();
    ImGuiTableFlags table_flags = 
        ImGuiTableFlags_PreciseWidths
        |ImGuiTableFlags_NoHostExtendX
        |ImGuiTableFlags_Borders;
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{0.7f, 0.7f, 0.7f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4{0.7f, 0.7f, 0.7f, 1.0f});
    if (ImGui::BeginTable("##condition code flags", 12, table_flags)) {
        ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_WidthFixed;
        auto text_width = 0.0f;
        ImGui::TableSetupColumn("N", column_flags, text_width);
        ImGui::TableSetupColumn("Z", column_flags, text_width);
        ImGui::TableSetupColumn("C", column_flags, text_width);
        ImGui::TableSetupColumn("V", column_flags, text_width);
        ImGui::TableSetupColumn("I", column_flags, text_width);
        ImGui::TableSetupColumn("F", column_flags, text_width);
        ImGui::TableSetupColumn("T", column_flags, text_width);
        ImGui::TableSetupColumn("M4", column_flags, text_width);
        ImGui::TableSetupColumn("M3", column_flags, text_width);
        ImGui::TableSetupColumn("M2", column_flags, text_width);
        ImGui::TableSetupColumn("M1", column_flags, text_width);
        ImGui::TableSetupColumn("M0", column_flags, text_width);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        for (int i = 31; i >= 28; --i) {
            ImGui::TableSetColumnIndex(31 - i);
            ImGui::TextFmt("{}", psr.val >> i & 1);
        }
        for (int i = 7; i >= 0; --i) {
            ImGui::TableSetColumnIndex(11 - i);
            ImGui::TextFmt("{}", psr.val >> i & 1);
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(2);
    ImGui::TextFmt("State: {}", psr.is_thumb() ? "Thumb" : "Arm");
    auto const operation_mode_str = [=] -> std::string {
        switch (psr.get_mode()) {
            case fgba::cpu::mode::usr: return "User";
            case fgba::cpu::mode::fiq: return "FIQ";
            case fgba::cpu::mode::irq: return "IRQ";
            case fgba::cpu::mode::svc: return "Supervisor";
            case fgba::cpu::mode::abt: return "Abort";
            case fgba::cpu::mode::sys: return "System";
            case fgba::cpu::mode::und: return "Undefined";
            default: return "???";
        }
    }();
    ImGui::TextFmt("Operation mode: {}", operation_mode_str);
    ImGui::TextFmt("FIQ: {}", psr.is_fiq_disabled() ? "disabled" : "enabled");
    ImGui::TextFmt("IRQ: {}", psr.is_irq_disabled() ? "disabled" : "enabled");
    ImGui::EndGroup();
}

void some(fgba::cpu::register_manager const& rm) {
    ImGuiWindowFlags wflags = 
        ImGuiWindowFlags_NoScrollbar
        |ImGuiWindowFlags_NoScrollWithMouse
        |ImGuiWindowFlags_NoCollapse
        |ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowSize(ImVec2{400, 400});
    if (not ImGui::Begin("cpu state", nullptr, wflags)) { return; }
    std::array items{
        "bin",
        "oct",
        "dec",
        "hex"
    };
    static int selected = 3;
    ImGuiTableFlags table_flags = 
        ImGuiTableFlags_PreciseWidths
        |ImGuiTableFlags_NoHostExtendX
        |ImGuiTableFlags_Borders
        |ImGuiTableFlags_RowBg;
    // ImGui::BeginMenu("data");
    // ImGui::EndMenu();
    ImGui::BeginGroup();
    if (ImGui::BeginTable("active registers", 2, table_flags)) {
        ImGui::TableSetupColumn("registers", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("data", ImGuiTableColumnFlags_WidthFixed, 13.0f * 7);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(1);
        ImGui::Combo("", &selected, items.data(), items.size());
        
        std::string const format{
            [] -> std::string {
                switch (selected) {
                    case 0: return "{:#034b}";
                    case 1: return "{:#018o}";
                    case 2: return "{:d}";
                    case 3: return "{:#010x}";
                    default: assert(false); return "";
                }
            }()
        };
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4());
        for (size_t i = 0; i < 14; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("r{}", i);
            ImGui::TableSetColumnIndex(1);
            ImGui::vTextFmt(format, rm[i]);
        }
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("lr");
        ImGui::TableSetColumnIndex(1);
        ImGui::vTextFmt(format, rm[14]);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextFmt("pc");
        ImGui::TableSetColumnIndex(1);
        ImGui::vTextFmt(format, rm[15]);
        ImGui::PopStyleColor(2);
        ImGui::EndTable();
    } 
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SeparatorText("Current program status register");
    display_psr(rm.cpsr());
    ImGui::EndGroup();
    ImGui::End();
}
int main(int argc, char* argv[]) try {
    // Setup Dear ImGui context
    using new_type = int;
    unsigned other_var{0};
    int& smth = reinterpret_cast<new_type&>(other_var);


    fgba::gameboy_advance gba; 
    glfwInit();
#ifndef NDEBUG
    gladSetGLPostCallback(post_call_gl_callback);
    gladSetGLPreCallback(pre_call_gl_callback);
#endif
    window_ptr window = create_window();
    glfwMakeContextCurrent(window.get());
    fmt::println("{}", std::filesystem::current_path().c_str());
    if (not window) {
        spdlog::error("failed to create a window");
        return -1;
    }
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        spdlog::error("failed to initialize opengl context");
        return -1;
    }
    spdlog::info("loaded opengl {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls 

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window.get(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    int widthf, heightf;
    glfwGetFramebufferSize(window.get(), &widthf, &heightf);
    glViewport(0, 0, widthf, heightf);

    [[maybe_unused]]glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
    [[maybe_unused]]ImFont* font = io.Fonts->AddFontFromFileTTF("../asset/fonts/ComicShannsMono/ComicShannsMonoNerdFontMono-Regular.otf", 14.0);
    // ImGui::PushFont(font);

    while (not glfwWindowShouldClose(window.get())) {
        glfwWaitEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        #ifdef IMGUI_HAS_VIEWPORT
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        #else 
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        #endif
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        static bool smth = 0;
        ImGuiWindowFlags const godwindowflags =
             ImGuiWindowFlags_NoDecoration 
            |ImGuiWindowFlags_NoResize 
            |ImGuiWindowFlags_MenuBar 
            |ImGuiWindowFlags_NoBringToFrontOnFocus
            |ImGuiWindowFlags_NoScrollWithMouse;
        if (ImGui::Begin("ayoo?", &smth, godwindowflags)){
            ImGui::BeginMenuBar();
            static bool display_cpu_data = 0;
            static bool exit = 0;
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("Open", nullptr, &display_cpu_data);
                if (ImGui::BeginMenu("Open Recent")) {
                    ImGui::MenuItem("some_smart_shit.cpp", nullptr, &display_cpu_data);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                ImGui::MenuItem("Save", nullptr, &display_cpu_data);
                ImGui::MenuItem("Save as", nullptr, &display_cpu_data);
                ImGui::MenuItem("Save all", nullptr, &display_cpu_data, false);
                ImGui::EndMenu();
            } 
            if (ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Undo", "Cmd+Z", &display_cpu_data);
                ImGui::MenuItem("Redo", "Cmd+Shift+Z", &display_cpu_data);
                ImGui::Separator();
                ImGui::MenuItem("Cut", "Cmd+X", &display_cpu_data);
                ImGui::MenuItem("Copy", "Cmd+C", &display_cpu_data);
                ImGui::MenuItem("Paste", "Cmd+V", &display_cpu_data);
                ImGui::EndMenu();
            } 
            if (ImGui::BeginMenu("Tools")) {
                ImGui::MenuItem("cpu data", nullptr, &display_cpu_data);
                ImGui::EndMenu();
            } 
            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("cpu data", nullptr, &display_cpu_data);
                ImGui::EndMenu();
            } 
            if (ImGui::BeginMenu("Help")) {
                ImGui::MenuItem("cpu data", nullptr, &display_cpu_data);
                ImGui::EndMenu();
            } 
            ImGui::EndMenuBar();
            if (display_cpu_data) {
                some(gba.dump_cpu_state().get_regitsters_contents());
            }
            ImGui::BeginChild("");
            // ImGui::ShowDemoWindow();
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar();
        
        
        // glClear(GL_COLOR_BUFFER_BIT);


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window.get());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
} catch (fgba::runtime_error const& e) {
    spdlog::error("{}", e);
}

[[nodiscard]]auto create_window() 
    -> window_ptr {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif
    return window_ptr{glfwCreateWindow(width, height, "[debug] fgba", nullptr, nullptr)};
}

auto create_projection()
    -> void {

}