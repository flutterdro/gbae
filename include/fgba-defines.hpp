#ifndef FGBA_DEFINES_HPP_OOOGA_BOOOOOOOOGA
#define FGBA_DEFINES_HPP_OOOGA_BOOOOOOOOGA

#include <cstddef>
#include <cstdint>

using u8 = std::uint8_t;
using i8 = std::int8_t;

using u16 = std::uint16_t;
using i16 = std::int16_t;

using u32 = std::uint32_t;
using i32 = std::int32_t;

using usize = std::size_t;

consteval auto operator""_u32(unsigned long long i) 
    -> u32 { return static_cast<u32>(i); }
consteval auto operator""_u16(unsigned long long i) 
    -> u16 { return static_cast<u16>(i); }
consteval auto operator""_u8(unsigned long long i) 
    -> u8 { return static_cast<u8>(i); }
consteval auto operator""_i32(unsigned long long i) 
    -> i32 { return static_cast<i32>(i); }
consteval auto operator""_i16(unsigned long long i) 
    -> i16 { return static_cast<i16>(i); }
consteval auto operator""_i8(unsigned long long i) 
    -> i8 { return static_cast<i8>(i); }



#endif