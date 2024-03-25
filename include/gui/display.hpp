#ifndef DISPLAY_HPP_SWNKAJQLJQNDKC
#define DISPLAY_HPP_SWNKAJQLJQNDKC

#include <mdspan>
#include "emulator/cpudefines.hpp"
#include "glad/gl.h"

namespace fgba::gui {

class display {
public:
    display(lcd_display_view) noexcept;
    display(display const&) = delete;
    display(display&&) noexcept;
    auto operator=(display const&) 
        -> display& = delete;
    auto operator=(display&&) noexcept 
        -> display&;
    ~display();

    auto update() const noexcept 
        -> void;
    [[nodiscard]]auto get_handle() const noexcept 
        -> GLuint;
private:
    lcd_display_view m_view;
    GLuint m_texture_id;
};

}

#endif