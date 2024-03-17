#include "ppu.hpp"
#include "cpudefines.hpp"
#include <__ranges/transform_view.h>
#include <bit>
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
auto ppu::get_display_view() const noexcept
    -> lcd_display_view {
    return lcd_display_view{m_display.data()};
}
auto ppu::mode3() noexcept
    -> void {
    auto bitmap_view = m_vram_view.subspan<0, 0x14000>();
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