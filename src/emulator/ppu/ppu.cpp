#include "emulator/ppu/ppu.hpp"
#include "emulator/cpudefines.hpp"
#include <ranges>
#include <bit>
#include <cstddef>
#include <cstring>
#include <span>

namespace fgba {
namespace  {

struct color {
    std::byte red;
    std::byte green;
    std::byte blue;
};
static_assert(alignof(color) == 1);
[[nodiscard]] constexpr auto gbacolor_to_rgb(u16 color16) 
    -> color {
    return {
        .red   = static_cast<std::byte>(8 * (color16 & 0x1f)),
        .green = static_cast<std::byte>(8 * ((color16 >> 5) & 0x1f)),
        .blue  = static_cast<std::byte>(8 * ((color16 >> 10)& 0x1f)),
    };
}

} // anonymous namesapace

namespace ppu {
ppu::ppu() {
    auto vram_view = std::span{m_vram};
    size_t counter = 0;
    unsigned color_offset = 0;
    for (size_t i = 0; i < 160; ++i) {
        for (size_t j = 0; j < 240; ++j) {
            m_display[i*240*3 + j*3 +0] = static_cast<std::byte>(color_offset);
            m_display[i*240*3 + j*3 +1] = static_cast<std::byte>(color_offset);
            m_display[i*240*3 + j*3 +2] = static_cast<std::byte>(color_offset);
        }
        color_offset += 1;
    }
}

auto ppu::get_display_view() const noexcept
    -> lcd_display_view {
    return lcd_display_view{m_display.data()};
}
auto ppu::mode3() noexcept
    -> void {
    auto bitmap_view = std::span<std::byte, 0x14000>{m_vram};
    //Just for now
    for (u32 i = 0; i < bitmap_view.size(); i += 2) {
        auto [red, green, blue] = gbacolor_to_rgb(mmu::read<u16>(bitmap_view, i));
        m_display[3 * i]     = red;
        m_display[3 * i + 1] = green;
        m_display[3 * i + 2] = blue;
    }
}
} // namespace ppu

} // namespace fgba
