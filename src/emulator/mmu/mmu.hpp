#ifndef GBA_MMU_H_
#define GBA_MMU_H_
#include <cstddef>
#include <fstream>
#include <iostream>
#include <source_location>

#include "../cpudefines.hpp"
#include "../utility/fatexception.hpp"
#include "memoryprimitives.hpp"

namespace fgba::mmu {
enum class memory_region_type : u32 {
    sysrom = 0x00,
    ewram,
    iwram,
    ioregisters,
    //internal display memory
    paletteram,
    vram,
    oam,
    //external memory (game pak)
    flashrom0,
    flashrom1,
    flashrom2,
    sram,
    invalid,
};
template<size_t lower_bound, size_t upper_bound, size_t bus_size>
class mem_owner {
public:
    [[nodiscard]] auto operator[](u32 address) const 
        -> std::byte;
private:
static constexpr size_t memsize = upper_bound - lower_bound + 1;
std::unique_ptr<std::array<std::byte, memsize>> m_mem_ptr;
};
class memory_managment_unit {
public:
    memory_managment_unit();
    memory_managment_unit(memory_managment_unit const&) = delete;
    auto operator=(memory_managment_unit const&)
        -> memory_managment_unit& = delete;
    memory_managment_unit(memory_managment_unit&&) noexcept = default;
    auto operator=(memory_managment_unit&&) noexcept
        -> memory_managment_unit& = default;

    template<size_t bus_size>
    [[nodiscard]]auto read(address) const
        -> bus_data<bus_size>;
    template<size_t bus_size>
    auto write(u32 address, bus_data<bus_size> data)
        -> void;

    auto load_bios(std::filesystem::path const& path) 
        -> void;
    auto load_gamerom(std::filesystem::path const& path_to_cartridge)
        -> void;
private:
    std::array<mem_region, 11> m_memory;
    readonlymem <0x00000000, 0x00003fff> m_bios;
    readwritemem<0x02000000, 0x0203ffff> m_ewram;
    readwritemem<0x03000000, 0x03007fff> m_iwram;
    memmap      <0x04000000, 0x040003fe> m_io_registers;
    readwritemem<0x05000000, 0x050003ff> m_pram;
    readwritemem<0x06000000, 0x06017fff> m_vram;
    readwritemem<0x07000000, 0x070003ff> m_oam;
    readonlymem <0x08000000, 0x09ffffff> m_gprom0;
    readonlymem <0x0a000000, 0x0bffffff> m_gprom1;
    readonlymem <0x0c000000, 0x0dffffff> m_gprom2;
    readwritemem<0x0e000000, 0x0e00ffff> m_sram;
};
}

#endif