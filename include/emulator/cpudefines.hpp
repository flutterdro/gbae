#ifndef FGBA_DEFINES_HPP
#define FGBA_DEFINES_HPP
#include <bit>
#include <mdspan>
#include <cstring>
#include <cstddef>
#include <type_traits>
#include "fgba-defines.hpp"
#include "utility/funky-ints.hpp"
//Utility header where helpers and aliases are defined

namespace fgba {
using word = funky<u32>;
consteval auto operator""_word(unsigned long long num)
    -> word { return word{.value = static_cast<u32>(num)}; }
using dword = funky<u64>;
consteval auto operator""_dword(unsigned long long num)
    -> dword { return dword{.value = static_cast<u64>(num)}; }
using hword = funky<u16>;
using byte  = funky<u8>;

namespace cpu::arm   { struct instruction : word  {}; }
namespace cpu::thumb { struct instruction : hword {}; }

template<typename T>
struct half_width;
template<>
struct half_width<u32> { using type = u16; };
template<>
struct half_width<u16> { using type = u8; };
template<typename T>
using half_width_t = typename half_width<T>::type;
using lcd_display_view = std::mdspan<std::byte const, std::extents<size_t, 3, 240, 160>>;

inline constexpr std::byte uninitialized_byte{0xeb};
inline constexpr u32 unitialized_word{0xebebebeb};


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
template<typename E>
consteval auto enum_size() -> size_t { return E::size; }
enum class shifts : unsigned {
    null = 0,
    lsr32,
    asr32,
    rrx,

    lsl,
    lsr,
    asr,
    ror,

    rslsl,
    rslsr,
    rsasr,
    rsror,

    count,
};

enum class data_size {
    word,
    hword,
    byte,

    count,
};

struct address : word {};

enum registers {
    r0 = 0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, lr, pc,
    r0_usr = r0, r1_usr = r1, r2_usr = r2, r3_usr = r3, r4_usr = r4, r5_usr = r5, r6_usr = r6, r7_usr = r7, r8_usr = r8, r9_usr = r9, r10_usr = r10, r11_usr = r11, r12_usr = r12, r13_usr = r13, lr_usr = lr, pc_usr = pc,
    r0_sys = r0, r1_sys = r1, r2_sys = r2, r3_sys = r3, r4_sys = r4, r5_sys = r5, r6_sys = r6, r7_sys = r7, r8_sys = r8, r9_sys = r9, r10_sys = r10, r11_sys = r11, r12_sys = r12, r13_sys = r13, lr_sys = lr, pc_sys = pc,
    r0_fiq = r0, r1_fiq = r1, r2_fiq = r2, r3_fiq = r3, r4_fiq = r4, r5_fiq = r5, r6_fiq = r6, r7_fiq = r7, r8_fiq = r8 + 8, r9_fiq = r9 + 8, r10_fiq = r10 + 8, r11_fiq = r11 + 8, r12_fiq = r12 + 8, r13_fiq = r13 + 8, lr_fiq = lr + 8, pc_fiq = pc,
    r0_svc = r0, r1_svc = r1, r2_svc = r2, r3_svc = r3, r4_svc = r4, r5_svc = r5, r6_svc = r6, r7_svc = r7, r8_svc = r8, r9_svc = r9, r10_svc = r10, r11_svc = r11, r12_svc = r12, r13_svc = r13 + 10, lr_svc = lr + 10, pc_svc = pc,
    r0_abt = r0, r1_abt = r1, r2_abt = r2, r3_abt = r3, r4_abt = r4, r5_abt = r5, r6_abt = r6, r7_abt = r7, r8_abt = r8, r9_abt = r9, r10_abt = r10, r11_abt = r11, r12_abt = r12, r13_abt = r13 + 12, lr_abt = lr + 12, pc_abt = pc,
    r0_irq = r0, r1_irq = r1, r2_irq = r2, r3_irq = r3, r4_irq = r4, r5_irq = r5, r6_irq = r6, r7_irq = r7, r8_irq = r8, r9_irq = r9, r10_irq = r10, r11_irq = r11, r12_irq = r12, r13_irq = r13 + 14, lr_irq = lr + 14, pc_irq = pc,
    r0_und = r0, r1_und = r1, r2_und = r2, r3_und = r3, r4_und = r4, r5_und = r5, r6_und = r6, r7_und = r7, r8_und = r8, r9_und = r9, r10_und = r10, r11_und = r11, r12_und = r12, r13_und = r13 + 16, lr_und = lr + 16, pc_und = pc,
};


}
}

#endif
