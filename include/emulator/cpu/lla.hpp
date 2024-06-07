#ifndef LOW_LEVEL_ARITHM_HPP_
#define LOW_LEVEL_ARITHM_HPP_
//low level arithmethic implementation 
//it relies on a lot of intrinsics

#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
namespace fgba::cpu {
inline auto add_impl(word operand1, word operand2, word* res, u32) noexcept
    -> bool {
#if __has_builtin(__builtin_add_overflow)
    return __builtin_uadd_overflow(operand1.value, operand2.value, &res->value);
#endif
}
inline auto adc_impl(word operand1, word operand2, word* res, u32 carryin) noexcept
    -> bool {
    u32 carryout; // NOLINT
#if __has_builtin(__builtin_add_overflow)
    res->value = __builtin_addc(operand1.value, operand2.value, carryin, &carryout);
    return carryout != 0u;
#endif
}
inline auto sub_impl(word operand1, word operand2, word* res, u32) noexcept
    -> bool {
#if __has_builtin(__builtin_add_overflow)
    return not __builtin_sub_overflow(operand1.value, operand2.value, &res->value);
#endif
}
inline auto sbc_impl(word operand1, word operand2, word* res, u32 carryin) noexcept
    -> bool {
    u32 carryout;
#if __has_builtin(__builtin_add_overflow)
    res->value = __builtin_addc(operand1.value, ~operand2.value, carryin, &carryout);
    return carryout != 0u;
#endif
}
inline auto rsb_impl(word operand1, word operand2, word* res, u32) noexcept
    -> bool {
    return sub_impl(operand2, operand1, res, 0); //NOLINT
}
inline auto rsc_impl(word operand1, word operand2, word* res, u32 carryin) noexcept
    -> bool {
    return sbc_impl(operand2, operand1, res, carryin); //NOLINT
}

}


#endif
