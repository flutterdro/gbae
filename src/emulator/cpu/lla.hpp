#ifndef LOW_LEVEL_ARITHM_HPP_
#define LOW_LEVEL_ARITHM_HPP_
//low level arithmethic implementation 
//it relies on a lot of intrinsics

#include "cpudefines.hpp"
namespace fgba::cpu {
inline auto add_impl(u32 operand1, u32 operand2, u32* res, u32) noexcept
    -> bool {
#if __has_builtin(__builtin_add_overflow)
    return __builtin_uadd_overflow(operand1, operand2, res);
#endif
}
inline auto addc_impl(u32 operand1, u32 operand2, u32* res, u32 carryin) noexcept
    -> bool {
    u32 carryout;
#if __has_builtin(__builtin_add_overflow)
    *res = __builtin_addc(operand1, operand2, carryin, &carryout);
    return carryout;
#endif
}

}


#endif