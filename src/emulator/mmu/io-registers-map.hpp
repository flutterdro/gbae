#ifndef IO_REGISTERS_MAP_HPP_DJINE_
#define IO_REGISTERS_MAP_HPP_DJINE_

#include "../ppu/registers.hpp"
#include "ppu/ppu.hpp"
#include "memoryprimitives.hpp"

namespace fgba::mmu {

class io_registers_map {
public: 
    using mem_spec = mem_spec<bounds<0x04000000, 0x04000400>, mem_type::ram, bus_size::word>;
    io_registers_map(ppu::ppu& ppu)
        : m_ppu{ppu} {}
    template<typename T>
    auto read(u32 address) const -> T { return address; }
private:
    ppu::ppu& m_ppu;
};


}

#endif