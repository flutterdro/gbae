#include "mmu.hpp"
#include "cpudefines.hpp"
#include "fatexception.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <algorithm>
#include <source_location>


namespace fgba::mmu {
constexpr std::array<int, 16> internal_region_mapping{
    0, -1, 1, 2, 3, 4, 5, 6, 7, 7, 8, 8, 9, 9, 10, -1 
};

auto get_region_type(address address) 
    -> memory_region_type {
    using enum memory_region_type;
    constexpr std::array<memory_region_type, 16> region_mapping {
        sysrom, invalid, ewram, iwram, ioregisters, paletteram, 
        vram, oam, flashrom0, flashrom0, flashrom1, flashrom1,
        flashrom2, flashrom2, sram, invalid, 
    };
    return region_mapping[(address >> 24) & 0xf];
}

struct bounds {
    u32 upper;
    u32 lower;
};

constexpr std::array<bounds, 11> boundaries {
    bounds{0x00003FFF, 0x00000000}, bounds{0x0203FFFF, 0x02000000}, 
    bounds{0x03007FFF, 0x03000000}, bounds{0x040003FE, 0x04000000}, 

    bounds{0x050003FF, 0x05000000}, bounds{0x06017FFF, 0x06000000}, bounds{0x070003FF, 0x07000000},

    bounds{0x09FFFFFF, 0x08000000}, bounds{0x0BFFFFFF, 0x0A000000}, 
    bounds{0x0DFFFFFF, 0x0C000000}, bounds{0x0E00FFFF, 0x0E000000}
};

memory_managment_unit::memory_managment_unit() 
    : m_memory{
        std::vector(0x00003FFF - 0x00000000 + 1, uninitialized_byte),
        std::vector(0x0203FFFF - 0x02000000 + 1, uninitialized_byte),
        std::vector(0x03007FFF - 0x03000000 + 1, uninitialized_byte),
        std::vector(0x040003FE - 0x04000000 + 1, uninitialized_byte),
        std::vector(0x050003FF - 0x05000000 + 1, uninitialized_byte),
        std::vector(0x06017FFF - 0x06000000 + 1, uninitialized_byte),
        std::vector(0x070003FF - 0x07000000 + 1, uninitialized_byte),
        std::vector(0x09FFFFFF - 0x08000000 + 1, uninitialized_byte),
        std::vector(0x0BFFFFFF - 0x0A000000 + 1, uninitialized_byte),
        std::vector(0x0DFFFFFF - 0x0C000000 + 1, uninitialized_byte),
        std::vector(0x0E00FFFF - 0x0E000000 + 1, uninitialized_byte),
    } {}
template<>
auto memory_managment_unit::read<8>(address address) const 
    -> bus_data<8> {
    bus_data<8> ret;
    memory_region_type const memreg = get_region_type(address);
    auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
    ret[0] = m_memory[std::to_underlying(memreg)][address - lower];
    return ret;
}
template<>
auto memory_managment_unit::read<16>(address address) const 
    -> bus_data<16> {
    bus_data<16> ret;
    memory_region_type const memreg = get_region_type(address);
    auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
    if (address + 2 > upper) {
        throw fgba::runtime_error{
            fmt::format("out of bound read at address: {}", address), 
            std::source_location::current()
        };
    }
    auto&& reg = m_memory[std::to_underlying(memreg)]
        |stdv::drop(address - lower)
        |stdv::take(2);
    stdr::copy(reg, ret.begin());
    return ret;
}
template<>
auto memory_managment_unit::read<32>(address const address) const 
    -> bus_data<32> {
    bus_data<32> ret;
    memory_region_type const memreg = get_region_type(address);
    switch (memreg) {
    case memory_region_type::sysrom:      [[fallthrough]];
    case memory_region_type::iwram:       [[fallthrough]];
    case memory_region_type::ioregisters: [[fallthrough]];
    case memory_region_type::oam: {
        auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
        if (address + 4 > upper) {
            throw fgba::runtime_error{
                fmt::format("out of bound read at address: {}", address), 
                std::source_location::current()
            };
        }
        auto&& reg = m_memory[std::to_underlying(memreg)]
            |stdv::drop(address - lower)
            |stdv::take(4);
        stdr::copy(reg, ret.begin());
    }
    case memory_region_type::ewram:       [[fallthrough]];
    case memory_region_type::paletteram:  [[fallthrough]];
    case memory_region_type::vram:        [[fallthrough]];
    case memory_region_type::flashrom0:   [[fallthrough]];
    case memory_region_type::flashrom1:   [[fallthrough]];
    case memory_region_type::flashrom2: {
        bus_data<16> lo = read<16>(address);
        bus_data<16> hi = read<16>(address + 2);
        ret = merge_buses<16>(lo, hi);
    }
    default: throw fgba::runtime_error{
        "atempted to read a word from sram",
        std::source_location::current()};
    }
    return ret;
}

template<>
auto memory_managment_unit::write<8>(address address, bus_data<8> data) 
    -> void {
    memory_region_type const memreg = get_region_type(address);
    auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
    m_memory[std::to_underlying(memreg)][address - lower] = data[0];
}
template<>
auto memory_managment_unit::write<16>(address address, bus_data<16> data) 
    -> void {
    memory_region_type const memreg = get_region_type(address);
    auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
    auto&& reg = m_memory[std::to_underlying(memreg)]
        |stdv::drop(address - lower);
    stdr::copy(data, reg.begin());
}
template<>
auto memory_managment_unit::write<32>(address address, bus_data<32> data) 
    -> void {
    memory_region_type const memreg = get_region_type(address);
    switch (memreg) {
    case memory_region_type::sysrom:      [[fallthrough]];
    case memory_region_type::iwram:       [[fallthrough]];
    case memory_region_type::ioregisters: [[fallthrough]];
    case memory_region_type::oam: {
        auto&& [upper, lower] = boundaries[std::to_underlying(memreg)];
        auto&& reg = m_memory[std::to_underlying(memreg)]
            |stdv::drop(address - lower);
        stdr::copy(data, reg.begin());
    }
    case memory_region_type::ewram:       [[fallthrough]];
    case memory_region_type::paletteram:  [[fallthrough]];
    case memory_region_type::vram:        [[fallthrough]];
    case memory_region_type::flashrom0:   [[fallthrough]];
    case memory_region_type::flashrom1:   [[fallthrough]];
    case memory_region_type::flashrom2: {
        auto&& [lo, hi] = split_bus<32>(data);
        write<16>(address, lo);
        write<16>(address + 2, hi);
    }
    default: 
      break;
    }
}


}