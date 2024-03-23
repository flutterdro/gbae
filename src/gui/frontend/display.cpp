#include "display.hpp"
#include "cpudefines.hpp"
#include "glad/gl.h"
#include <__expected/expected.h>
#include <__expected/unexpected.h>
#include <expected>

namespace fgba::gui {

display::display(lcd_display_view view) noexcept
    : m_view{view} {
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
        static_cast<int>(m_view.extent(1)), // cast away the warning just as god inteneded
        static_cast<int>(m_view.extent(2)), // jokes aside it is fine since display is 240x160
        0, GL_RGB, GL_UNSIGNED_BYTE, m_view.data_handle()
    );
}
display::display(display&& other) noexcept
    : m_view{other.m_view}, m_texture_id{other.m_texture_id} {
    other.m_texture_id = 0;
} 
auto display::operator=(display&& other) noexcept
    -> display& {
    m_view = other.m_view;
    m_texture_id = other.m_texture_id;
    other.m_texture_id = 0;

    return *this;
}
display::~display() {
    glDeleteTextures(1, &m_texture_id);
}
auto display::update() noexcept
    -> void {
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
        static_cast<int>(m_view.extent(1)), 
        static_cast<int>(m_view.extent(2)), 
        GL_RGB, GL_UNSIGNED_BYTE, m_view.data_handle()
    );
}

auto display::get_handle() noexcept
    -> GLuint { return m_texture_id; }

}