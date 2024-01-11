
#include <exception>
#include <filesystem>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ranges>
#include <expected>
#include <span>

#include <fmt/core.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "backend/shader.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "utility/fatexception.hpp"
#include "emulator/gbaemu.hpp"
#include "emulator/cpu/disassembler/disassembler.hpp"

constexpr GLuint width = 800;
constexpr GLuint height = 600;

using window_ptr = std::unique_ptr<GLFWwindow, decltype([](auto& a) {glfwDestroyWindow(a);})>;

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

int main(int argc, char* argv[]) try {
    // Setup Dear ImGui context

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

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    while (not glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        glClear(GL_COLOR_BUFFER_BIT);


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