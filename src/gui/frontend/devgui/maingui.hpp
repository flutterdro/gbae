#ifndef MAIN_GUI_HPP_DNJCEIJNA
#define MAIN_GUI_HPP_DNJCEIJNA

#include "../../../emulator/gbaemu.hpp"
#include "frontend/display.hpp"
#include "GLFW/glfw3.h"
#include "cpu/registermanager.hpp"
#include <expected>

namespace fgba::gui {
struct window_destroyer {
    auto operator()(GLFWwindow* ptr) -> void { glfwDestroyWindow(ptr); }
};
using window_ptr = std::unique_ptr<GLFWwindow, window_destroyer>;
// My error type is string and I am proud of it
struct frame {
    [[nodiscard]]frame(GLFWwindow*);
    ~frame();
private:
    GLFWwindow* m_window;
};
auto setup_opengl_context() -> std::expected<window_ptr, std::string>; 
auto setup_imgui(GLFWwindow* ctx) -> void;

auto draw_main_gui(gameboy_advance& gba) -> void;
auto draw_cpu_registers_dump(cpu::register_manager const& rm) -> void;
auto draw_emu_display(display const&) -> void;
}

#endif