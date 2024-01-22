#ifndef PICTURE_PROCESSING_UNIT_HPP_
#define PICTURE_PROCESSING_UNIT_HPP_

#include <bit>
#include <cstddef>
#include <span>

#include "cpudefines.hpp"

namespace fgba::ppu {
class ppu {
    struct dipscnt {
        u16 bg_mode                : 3;
        u16 cgb_mode               : 1;
        u16 frame                  : 1;
        u16 hblank_oam_access      : 1;
        u16 obj_vram_mapping       : 1;
        u16 forced_blank           : 1;
        u16 screen_display_bg0     : 1;
        u16 screen_display_bg1     : 1;
        u16 screen_display_bg2     : 1;
        u16 screen_display_bg3     : 1;
        u16 screen_display_obj     : 1;
        u16 window0_display_flag   : 1;
        u16 window1_display_flag   : 1;
        u16 objwindow_display_flag : 1;
        [[nodiscard]] auto to_u16() const noexcept 
            -> u16 { return std::bit_cast<u16>(*this); } 
    };
    static_assert(sizeof(dipscnt) == sizeof(u16), "Dipscnt must be 16 bit wide\n"
    "Your compiler seems to handle bitfields in a... quirky way. Try a different compiler");

private:
    std::span<std::byte, 0x06017fff - 0x06000000 + 1> const m_vram_view;
    dipscnt m_lcd_control;
};
}

#endif


