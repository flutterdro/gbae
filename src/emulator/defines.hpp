#ifndef FGBA_DEFINES_HPP
#define FGBA_DEFINES_HPP
#include <utility>
#include <cstddef>
#include <memory>
#include <ranges>
#include <vector>

namespace fgba {
template<size_t size>
using readonly_ptr = std::unique_ptr<std::array<std::byte const, size> const>;
template<size_t size>
using readwrite_ptr = std::unique_ptr<std::array<std::byte, size>>;
using mem_region = std::vector<std::byte>;
template<size_t bus_size>
    requires (bus_size % 8 == 0)
using bus_data = std::array<std::byte, bus_size/8>;
template<size_t bus_size, size_t...Is>
constexpr auto merge_buses_impl(bus_data<bus_size> const& lo, bus_data<bus_size> const& hi, 
    std::index_sequence<Is...>) noexcept
    -> bus_data<2 * bus_size> {
    return bus_data<2 * bus_size>{
        lo[Is]... ,
        hi[Is]...
    };
}
template<size_t bus_size>
constexpr auto merge_buses(bus_data<bus_size> const&  lo, bus_data<bus_size> const& hi) noexcept
    -> bus_data<2 * bus_size> {
    return merge_buses_impl<bus_size>(lo, hi, std::make_index_sequence<bus_size/8>{});
}
template<size_t bus_size, size_t...Is>
constexpr auto split_bus_impl(bus_data<bus_size> const& data, 
    std::index_sequence<Is...>) noexcept
    -> std::pair<bus_data<bus_size/2>, bus_data<bus_size/2>> {
    return {
        {data[Is]...} ,
        {data[sizeof...(Is) + Is]...}
    };
}
template<size_t bus_size>
constexpr auto split_bus(bus_data<bus_size> const& data) noexcept
    -> std::pair<bus_data<bus_size/2>, bus_data<bus_size/2>>{
    return split_bus_impl<bus_size>(data, std::make_index_sequence<bus_size/16>{});
}

using u32 = std::uint32_t;
consteval auto operator""_u32(unsigned long long i) 
    -> u32 { return static_cast<u32>(i); }
using u16 = std::uint16_t;
consteval auto operator""_u16(unsigned long long i) 
    -> u16 { return static_cast<u16>(i); }
using u8  = std::uint8_t;
consteval auto operator""_u8(unsigned long long i) 
    -> u8 { return static_cast<u8>(i); }
using i32 = std::int32_t;
consteval auto operator""_i32(unsigned long long i) 
    -> i32 { return static_cast<i32>(i); }
using i16 = std::int16_t;
consteval auto operator""_i16(unsigned long long i) 
    -> i16 { return static_cast<i16>(i); }
using i8  = std::int8_t;
consteval auto operator""_i8(unsigned long long i) 
    -> i8 { return static_cast<i8>(i); }
using address = u32;
constexpr std::byte uninitialized_byte{0xcd};
namespace stdr = std::ranges;
namespace stdv = std::views;

namespace cpu {
enum class mode : u32 {
    usr = 0b10000,
    fiq = 0b10001,
    irq = 0b10010,
    svc = 0b10011,
    abt = 0b10111,
    sys = 0b11111,
    und = 0b11011,
};

enum ccf : u32 {
    n = 1_u32 << 31,
    z = 1_u32 << 30,
    c = 1_u32 << 29,
    v = 1_u32 << 28,
};

enum arm_instruction_set {
    bx = 0, b, bl,
    //
    /// DATA PROCESSING
    //
    //and group
    andlsl0, andrrx, and_,
    andrs,
    andi,
    undefined,
};

}
}

template<typename T, unsigned B>
auto signextend(T const x) -> T {
    struct { T x:B; } s;
    return s.x = x;
}

#endif