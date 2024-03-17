#ifndef FGBA_CORE_HPP
#define FGBA_CORE_HPP


#include <bit>

#include "cpu/arm7tdmi.hpp"
#include "mmu/mmu.hpp"
#include "ppu/ppu.hpp"

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
private:
    cpu::arm7tdmi m_cpu;
    ppu::ppu m_ppu;
    mmu::memory_managment_unit m_mmu;
};
}

#endif