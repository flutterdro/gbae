#ifndef FGBA_CORE_HPP
#define FGBA_CORE_HPP


#include <bit>

#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpudefines.hpp"
#include "emulator/mmu/mmu.hpp"
#include "emulator/ppu/ppu.hpp"
#include <filesystem>

namespace fgba {
class gameboy_advance {
public:
    gameboy_advance();
    auto load_bios(std::filesystem::path const& path) 
        -> void {
        m_mmu.load_bios(path);
    }
    [[nodiscard]]auto dump_cpu_state() const noexcept
        -> cpu::arm7tdmi const& {
            return m_cpu;
        }
    [[nodiscard]]auto get_display_view() const noexcept
        -> lcd_display_view {
        return m_ppu.get_display_view();
    }
private:
    cpu::arm7tdmi m_cpu;
    ppu::ppu m_ppu;
    mmu::memory_managment_unit m_mmu;
};
}

#endif