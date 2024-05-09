#include "emulator/gbaemu.hpp"
#include "utility/fatexception.hpp"
#include "emulator/mmu/mmu.hpp"
#include <source_location>
namespace fgba {
static std::vector<std::byte> dummy;
gameboy_advance::gameboy_advance()
    : m_cpu{}, m_ppu{}, m_mmu{m_ppu} {
} 

}
