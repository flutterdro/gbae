#include "emulator/gbaemu.hpp"
#include "utility/fatexception.hpp"
#include "emulator/mmu/mmu.hpp"
#include <source_location>
namespace fgba {
static std::vector<std::byte> dummy;
gameboy_advance::gameboy_advance()
    : m_cpu{}, m_ppu{}, m_mmu{m_ppu} {
    m_cpu.conect_read(
        [&mmu = this->m_mmu](cpu::bus::signals signals) 
            -> u32 {
            switch (signals.mas) {
                case 0: return *mmu.read<u8>(signals.address);
                case 1: return *mmu.read<u16>(signals.address);
                case 2: return *mmu.read<u32>(signals.address);
            }
            return 0;
        }
    );
    m_cpu.conect_write(
        [&mmu = this->m_mmu](u32 data, cpu::bus::signals signals) 
            -> void {
            switch (signals.mas) {
                case 0: mmu.write<u8>(signals.address, static_cast<u8>(data));
                case 1: mmu.write<u16>(signals.address, static_cast<u16>(data));
                case 2: mmu.write<u32>(signals.address, data);
            }
        }
    );
} 

}