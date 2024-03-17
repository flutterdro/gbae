#ifndef IO_REGISTERS_HPP_IEKFJNCJEF_
#define IO_REGISTERS_HPP_IEKFJNCJEF_

#include "cpudefines.hpp"
namespace fgba::ppu {
template<typename T>
struct enable_read_operations {
    template<typename U, std::size_t Nth>
    [[nodiscard]]constexpr auto read() noexcept
        -> u8 {
        static_assert(sizeof(T) > Nth, "biting more than you can chew");
        return std::get<Nth>(std::bit_cast<std::array<U, sizeof(T) / sizeof(U)>>(static_cast<T&>(*this)));
    }
    template<std::size_t Nth>
    [[nodiscard]]constexpr auto read_byte() noexcept
        -> u8 {
        static_assert(sizeof(T) > Nth, "biting more than you can chew");
        return std::get<Nth>(std::bit_cast<std::array<u8, sizeof(T)>>(static_cast<T&>(*this)));
    }
    template<std::size_t Nth> 
    [[nodiscard]]constexpr auto read_halfword() const noexcept
        -> u16 requires (sizeof(T) >= 2) {
        static_assert(sizeof(T) / 2 > Nth, "biting more than you can chew");
        return std::get<Nth>(std::bit_cast<std::array<u16, sizeof(T) / 2>>(static_cast<T&>(*this)));
    }
    [[nodiscard]]constexpr auto read_word() const noexcept
        -> u32 requires (sizeof(T) == 4) {
        return std::bit_cast<u32>(static_cast<T&>(*this));
    }
};
template<typename T>
struct enable_write_operations {
    template<std::size_t Nth>
    constexpr auto write_byte(u8 data) noexcept
        -> void {
        static_assert(sizeof(T) > Nth, "biting more than you can chew");
        auto bytes = std::bit_cast<std::array<u8, sizeof(T)>>(static_cast<T&>(*this));
        std::get<Nth>(bytes) = data;
        static_cast<T&>(*this) = std::bit_cast<T>(bytes);
    }
    template<std::size_t Nth> 
    constexpr auto write_halfword(u16 data) const noexcept
        -> void requires (sizeof(T) >= 2) {
        static_assert(sizeof(T) / 2 > Nth, "biting more than you can chew");
        auto halfwords = std::bit_cast<std::array<u8, sizeof(T)/2>>(static_cast<T&>(*this));
        std::get<Nth>(halfwords) = data;
        static_cast<T&>(*this) = std::bit_cast<T>(halfwords);
    }
    constexpr auto write_word(u32 data) const noexcept
        -> void requires (sizeof(T) == 4) {
        static_cast<T&>(*this) = std::bit_cast<T>(data);
    }
};
constexpr u32 dispcnt_addr = 0x4000000;
struct dispcnt 
    : enable_read_operations<dispcnt>,
      enable_write_operations<dispcnt> {
    u16 bg_mode                : 3 = 0;
    u16 cgb_mode               : 1 = 0;
    u16 frame                  : 1 = 0;
    u16 hblank_oam_access      : 1 = 0;
    u16 obj_vram_mapping       : 1 = 0;
    u16 forced_blank           : 1 = 0;
    u16 screen_display_bg0     : 1 = 0;
    u16 screen_display_bg1     : 1 = 0;
    u16 screen_display_bg2     : 1 = 0;
    u16 screen_display_bg3     : 1 = 0;
    u16 screen_display_obj     : 1 = 0;
    u16 f_window0_display      : 1 = 0;
    u16 f_window1_display      : 1 = 0;
    u16 f_objwindow_display    : 1 = 0;
    constexpr dispcnt() noexcept = default;
};
constexpr u32 dispstat_addr = 0x4000004;
struct dispstat 
    : enable_read_operations<dispstat>, 
      enable_write_operations<dispstat> {
    u16 f_vblank        : 1 = 0;
    u16 f_hblank        : 1 = 0;
    u16 f_vcounter      : 1 = 0;
    u16 irq_vblank      : 1 = 0;
    u16 irq_hblank      : 1 = 0;
    u16 irq_vcounter    : 1 = 0;
    u16 not_used1       : 1 = 0;
    u16 not_used2       : 1 = 0;
    u16 vcount_setting  : 8 = 0;
};
constexpr u32 vcount_addr = 0x4000006;
struct vcount 
    : enable_read_operations<vcount> {
    u16 current_scanline : 8 = 0;
    u16 not_used_padding : 8 = 0;
};
constexpr u32 bg0cnt_addr = 0x4000008;
struct bg0cnt
    : enable_read_operations<bg0cnt>,
      enable_write_operations<bg0cnt> {
    u16 bg_priority             : 2 = 0;
    u16 character_base_block    : 2 = 0;
    u16 not_used                : 2 = 0;
    u16 mosaic                  : 1 = 0;
    u16 colors                  : 1 = 0;
    u16 not_used1               : 1 = 0;
    u16 screen_size             : 2 = 0;
};
constexpr u32 bg1cnt_addr = 0x400000a;
struct bg1cnt
    : enable_read_operations<bg1cnt>,
      enable_write_operations<bg1cnt> {
    u16 bg_priority             : 2 = 0;
    u16 character_base_block    : 2 = 0;
    u16 not_used                : 2 = 0;
    u16 mosaic                  : 1 = 0;
    u16 colors                  : 1 = 0;
    u16 not_used1               : 1 = 0;
    u16 screen_size             : 2 = 0;
};
constexpr u32 bg2cnt_addr = 0x400000c;
struct bg2cnt
    : enable_read_operations<bg2cnt>,
      enable_write_operations<bg2cnt> {
    u16 bg_priority             : 2 = 0;
    u16 character_base_block    : 2 = 0;
    u16 not_used                : 2 = 0;
    u16 mosaic                  : 1 = 0;
    u16 colors                  : 1 = 0;
    u16 display_area_overflow   : 1 = 0;
    u16 screen_size             : 2 = 0;
};
constexpr u32 bg3cnt_addr = 0x400000e;
struct bg3cnt
    : enable_read_operations<bg2cnt>,
      enable_write_operations<bg2cnt> {
    u16 bg_priority             : 2 = 0;
    u16 character_base_block    : 2 = 0;
    u16 not_used                : 2 = 0;
    u16 mosaic                  : 1 = 0;
    u16 colors                  : 1 = 0;
    u16 display_area_overflow   : 1 = 0;
    u16 screen_size             : 2 = 0;
};

}
#endif