#ifndef GBA_MMU_H_
#define GBA_MMU_H_
#include <cstddef>
#include <fstream>
#include <iostream>
#include <source_location>

#include "../cpudefines.hpp"
#include "../utility/fatexception.hpp"
#include "fmt/format.h"
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
    memory_managment_unit() = default;
    memory_managment_unit(memory_managment_unit const&) = delete;
    auto operator=(memory_managment_unit const&)
        -> memory_managment_unit& = delete;
    memory_managment_unit(memory_managment_unit&&) noexcept = default;
    auto operator=(memory_managment_unit&&) noexcept
        -> memory_managment_unit& = default;
    template<typename T>
    [[nodiscard]]auto read(u32 address) const noexcept
        -> std::optional<T> {
        if (address < m_bios.get_upper_bound() and address > m_bios.get_lower_bound()) {
            return m_bios.read<T>(address);
        } else if (address < m_ewram.get_upper_bound() and address > m_ewram.get_lower_bound()) {
            return m_ewram.read<T>(address);
        } else if (address < m_iwram.get_upper_bound() and address > m_iwram.get_lower_bound()) {
            return m_iwram.read<T>(address);
        } else if (address < m_io_registers.get_upper_bound() and address > m_io_registers.get_lower_bound()) {
            return m_io_registers.read<T>(address);
        } else if (address < m_pram.get_upper_bound() and address > m_pram.get_lower_bound()) {
            return m_pram.read<T>(address);
        } else if (address < m_vram.get_upper_bound() and address > m_vram.get_lower_bound()) {
            return m_vram.read<T>(address);
        } else if (address < m_oam.get_upper_bound() and address > m_oam.get_lower_bound()) {
            return m_oam.read<T>(address);
        } else if (address < m_gprom0.get_upper_bound() and address > m_gprom0.get_lower_bound()) {
            return m_gprom0.read<T>(address);
        } else if (address < m_gprom1.get_upper_bound() and address > m_gprom1.get_lower_bound()) {
            return m_gprom1.read<T>(address);
        } else if (address < m_gprom2.get_upper_bound() and address > m_gprom2.get_lower_bound()) {
            return m_gprom2.read<T>(address);
        } else if (address < m_sram.get_upper_bound() and address > m_sram.get_lower_bound()) {
            return m_sram.read<T>(address);
        } else {
            return std::nullopt;
        }
    }
    template<typename T>
    auto write(u32 address, T data)
        -> void {
        if (address < m_bios.get_upper_bound() and address > m_bios.get_lower_bound()) {
            throw fgba::runtime_error(fmt::format("Write of {1:#x} to read-only address: {0:#010x}", address, data));
        } else if (address < m_ewram.get_upper_bound() and address > m_ewram.get_lower_bound()) {
            m_ewram.write<T>(address, data);
        } else if (address < m_iwram.get_upper_bound() and address > m_iwram.get_lower_bound()) {
            m_iwram.write<T>(address, data);
        } else if (address < m_io_registers.get_upper_bound() and address > m_io_registers.get_lower_bound()) {
            m_io_registers.write<T>(address, data);
        } else if (address < m_pram.get_upper_bound() and address > m_pram.get_lower_bound()) {
            m_pram.write<T>(address, data);
        } else if (address < m_vram.get_upper_bound() and address > m_vram.get_lower_bound()) {
            m_vram.write<T>(address, data);
        } else if (address < m_oam.get_upper_bound() and address > m_oam.get_lower_bound()) {
            m_oam.write<T>(address, data);
        } else if (address < m_gprom0.get_upper_bound() and address > m_gprom0.get_lower_bound()) {
            throw fgba::runtime_error(fmt::format("Write of {1:#x} to read-only address: {0:#010x}", address, data));
        } else if (address < m_gprom1.get_upper_bound() and address > m_gprom1.get_lower_bound()) {
            throw fgba::runtime_error(fmt::format("Write of {1:#x} to read-only address: {0:#010x}", address, data));
        } else if (address < m_gprom2.get_upper_bound() and address > m_gprom2.get_lower_bound()) {
            throw fgba::runtime_error(fmt::format("Write of {1:#x} to read-only address: {0:#010x}", address, data));
        } else if (address < m_sram.get_upper_bound() and address > m_sram.get_lower_bound()) {
            m_sram.write<T>(address, data);
        } else {
            throw fgba::runtime_error(fmt::format("Write of {1:#x} to non-existent address: {0:#010x}", address, data));
        }
    }
    auto load_bios(std::filesystem::path const& path) 
        -> void;
    auto load_bios(readonlymem_view<>)
        -> void;
    template<std::invocable<void> F>
    auto map_read(u32 address, F&& func) 
        -> void { m_io_registers.bind_read(address, std::forward<F>(func)); }
    template<std::invocable<u8> F>
    auto map_write(u32 address, F&& func)
        -> void { m_io_registers.bind_write(address, std::forward<F>(func)); }
    auto load_gamerom(std::filesystem::path const& path_to_cartridge)
        -> void;
private:
    readonlymem <0x00000000, 0x00003fff> m_bios;
    readwritemem<0x02000000, 0x0203ffff> m_ewram;
    readwritemem<0x03000000, 0x03007fff> m_iwram;
    memmap      <0x04000000, 0x040003ff> m_io_registers;
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