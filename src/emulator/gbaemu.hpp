#ifndef FGBA_CORE_HPP
#define FGBA_CORE_HPP


#include <bit>

#include "cpu/arm7tdmi.hpp"
#include "mmu/mmu.hpp"

namespace fgba {
class gameboy_advance {
public:
    gameboy_advance();
    auto load_bios(std::filesystem::path const& path) 
        -> void {
        m_mmu.load_bios(path);
    }
private:
    cpu::arm7tdmi m_cpu;
    mmu::memory_managment_unit m_mmu;
};
}

#endif