#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "gui/frontend/devgui/maingui.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "scopeguard.hpp"
#include "utility/fatexception.hpp"
#include "emulator/gbaemu.hpp"

int main() try {

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


    while (not glfwWindowShouldClose(window.get())) {
        glfwWaitEvents();

        auto new_frame = fgba::gui::frame(window.get());

        fgba::gui::draw_main_gui(gba);
    }
} catch (fgba::runtime_error const& e) {
    spdlog::error("{}", e);
}