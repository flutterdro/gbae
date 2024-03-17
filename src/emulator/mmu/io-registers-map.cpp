#include "io-registers-map.hpp"

#define CASE_BYTE_READ_FOR_B(register)\
CASE_BYTE_READ_WITH_OFFSET(register, 0)
#define CASE_BYTE_READ_FOR_H(register)\
CASE_BYTE_READ_WITH_OFFSET(register, 0)\
CASE_BYTE_READ_WITH_OFFSET(register, 1)
#define CASE_BYTE_READ_FOR_W(register)\
CASE_BYTE_READ_WITH_OFFSET(register, 0)\
CASE_BYTE_READ_WITH_OFFSET(register, 1)\
CASE_BYTE_READ_WITH_OFFSET(register, 2)\
CASE_BYTE_READ_WITH_OFFSET(register, 3)
#define CASE_BYTE_READ_WITH_OFFSET(register, offset)\
case register##_addr + offset: {return m_ppu.m_##register.read<u8, offset>(); break;}

namespace fgba::mmu {
// template<>
// auto io_registers_map::read<u8>(u32 address) -> u8{
//     switch (address) {
//     using namespace ppu;
//         CASE_BYTE_READ_FOR_H(dispcnt)
//         CASE_BYTE_READ_FOR_H(dispstat)
//         CASE_BYTE_READ_FOR_H(vcount)
//     }
// }
}