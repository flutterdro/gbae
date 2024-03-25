#ifndef FGBA_DEFINES_HPP
#define FGBA_DEFINES_HPP
#include <__mdspan/extents.h>
#include <__mdspan/mdspan.h>
#include <cassert>
#include <cstring>
#include <functional>
#include <span>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <memory>
#include <ranges>
#include <vector>
#include <bit>
#include <algorithm>
//Utility header where helpers and aliases are defined

namespace fgba {
namespace stdr = std::ranges;
//to be completely portable I should've used uint_least32_t etc but it complicates things way too much
using u32 = std::uint32_t;
static_assert(sizeof(u32) == 4 * sizeof(std::byte), "God is dead");
consteval auto operator""_u32(unsigned long long i) 
    -> u32 { return static_cast<u32>(i); }
using u16 = std::uint16_t;
static_assert(sizeof(u16) == 2 * sizeof(std::byte), "God remains dead");
consteval auto operator""_u16(unsigned long long i) 
    -> u16 { return static_cast<u16>(i); }
using u8  = std::uint8_t;
static_assert(sizeof(u8) == sizeof(std::byte), "And we killed him");
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
template<typename T>
struct half_width;
template<>
struct half_width<u32> { using type = u16; };
template<>
struct half_width<u16> { using type = u8; };
template<typename T>
using half_width_t = typename half_width<T>::type;
using lcd_display_view = std::mdspan<std::byte const, std::extents<size_t, 3, 240, 160>>;

constexpr std::byte uninitialized_byte{0xeb};
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

enum arm_instruction_set {
    //
    /// BRANCHING
    //

    bx = 0, b, bl,

    //
    /// DATA PROCESSING
    //
#define GEN_VARIATIONS(name) \
    name ## lsr32, name ## asr32, name ## rrx, name ## lsl, name ## lsr, name ## asr, name ## ror, \
    name ## rslsl, name ## rslsr, name ## rsasr, name ## rsror, \
    name ## lsr32 ## s, name ## asr32 ## s, name ## rrx ## s, name ## lsl ## s, name ## lsr ## s, name ## asr ## s, name ## ror ## s, \
    name ## rslsl ## s, name ## rslsr ## s, name ## rsasr ## s, name ## rsror ## s, \
    name ## i, name ## is, name ## s, name
#define GEN_VARIATIONS_NO_S(name) \
    name ## lsr32, name ## asr32, name ## rrx, name ## lsl, name ## lsr, name ## asr, name ## ror, \
    name ## rslsl, name ## rslsr, name ## rsasr, name ## rsror, \
    name ## i, name
    /// LOGICAL GROUP

    // and
    GEN_VARIATIONS(and_),
    GEN_VARIATIONS(orr), 
    GEN_VARIATIONS(eor), 
    GEN_VARIATIONS(bic), 

    /// NO DESTINATION LOGICAL GROUP

    // tst
    GEN_VARIATIONS_NO_S(tst),
    GEN_VARIATIONS_NO_S(teq),

    /// ARITHMETIC GROUP
    GEN_VARIATIONS(add), 
    GEN_VARIATIONS(adc), 
    GEN_VARIATIONS(sub), 
    GEN_VARIATIONS(sbc), 
    GEN_VARIATIONS(rsb), 
    GEN_VARIATIONS(rsc), 

    /// NO DESTINATION ARITHMETIC GROUP
    GEN_VARIATIONS_NO_S(cmp),
    GEN_VARIATIONS_NO_S(cmn),

    /// SINGLE OPERAND GROUP

    // mov
    GEN_VARIATIONS(mov), 
    // movn
    GEN_VARIATIONS(mvn), 
    
#undef GEN_SHIFT_VARIATIONS
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