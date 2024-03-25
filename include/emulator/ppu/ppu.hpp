#ifndef PICTURE_PROCESSING_UNIT_HPP_
#define PICTURE_PROCESSING_UNIT_HPP_

#include <bit>
#include <cstddef>
#include <span>
#include <mdspan>
#include <sys/_types/_uintptr_t.h>

#include "emulator/cpudefines.hpp"
#include "emulator/ppu/registers.hpp"
#include "spdlog/spdlog.h"
#include "emulator/mmu/memoryprimitives.hpp"
namespace fgba::mmu {
class io_registers_map;
}
namespace fgba::ppu {
using pram_spec     = mem_spec<bounds<0x05000000, 0x05000400>, mem_type::ram, bus_size::hword>;
using vram_spec     = mem_spec<bounds<0x06000000, 0x06018000>, mem_type::ram, bus_size::hword>;
using oam_spec      = mem_spec<bounds<0x07000000, 0x07000400>, mem_type::ram, bus_size::hword>;
class ppu {
    friend class mmu::io_registers_map;
public:
    ppu();
    [[nodiscard]]auto get_display_view() const noexcept
        -> lcd_display_view;
    auto get_vram()
        -> mmu::mem_owner<vram_spec>& { return m_vram; }
    auto get_pram()
        -> mmu::mem_owner<pram_spec>& { return m_pram; }
    auto get_oam()
        -> mmu::mem_owner<oam_spec>& { return m_oam; }
private:
    auto mode3() noexcept
        -> void;
private:
    mmu::mem_owner<vram_spec> m_vram;
    mmu::mem_owner<pram_spec> m_pram;
    mmu::mem_owner<oam_spec> m_oam;
    std::array<std::byte, 240 * 160 * 3> m_display;
    dispcnt m_dispcnt;
    dispstat m_dispstat;
    vcount m_vcount;
};
}

#endif


