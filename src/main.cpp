#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "gui/devgui/maingui.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "utility/scopeguard.hpp"
#include "utility/fatexception.hpp"
#include "emulator/gbaemu.hpp"

auto main() -> int try {
     
    auto window = fgba::gui::window_ptr{nullptr};
    {
    auto window_exp = fgba::gui::setup_opengl_context();
    if (not window_exp.has_value()) {
        spdlog::error("{}", window_exp.error());
        return -1;
    }
    window = std::move(*window_exp);
    }
    fgba::gui::setup_imgui(window.get());
    fgba::scope_exit gui_guard {
        [] {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwTerminate();
        }
    };
    fgba::gameboy_advance gba;
    auto display = fgba::gui::display{gba.get_display_view()};


    while (glfwWindowShouldClose(window.get()) == 0) {
        glfwWaitEvents();
        auto new_frame = fgba::gui::frame(window.get());

        fgba::gui::draw_main_gui(gba, display);
    }
} catch (fgba::runtime_error const& e) {
    spdlog::error("{}", e);
}
