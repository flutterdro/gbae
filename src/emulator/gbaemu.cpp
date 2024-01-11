#include "gbaemu.hpp"
#include "../utility/fatexception.hpp"
#include "mmu/mmu.hpp"
#include <source_location>
namespace fgba {

gameboy_advance::gameboy_advance()
    : m_mmu{}, m_cpu{} {
    m_cpu.conect_read(
        [&mmu = this->m_mmu](cpu::bus::signals signals) 
            -> u32 {
            switch (signals.mas) {
                case 0: return std::bit_cast<u8>(mmu.read<8>(signals.address));
                case 1: return std::bit_cast<u16>(mmu.read<16>(signals.address));
                case 2: return std::bit_cast<u32>(mmu.read<32>(signals.address));
            }
            return 0;
        }
    );
    m_cpu.conect_write(
        [&mmu = this->m_mmu](u32 data, cpu::bus::signals signals) 
            -> void {
            switch (signals.mas) {
                case 0: mmu.write<8>(signals.address, std::bit_cast<bus_data<8>>(static_cast<u8>(data)));
                case 1: mmu.write<16>(signals.address, std::bit_cast<bus_data<16>>(static_cast<u16>(data)));
                case 2: mmu.write<32>(signals.address, std::bit_cast<bus_data<32>>(data));
            }
        }
    );
} 

}