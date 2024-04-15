#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <bit>
#include <type_traits>
#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
#include "registermanager.hpp"
namespace fgba::cpu {
namespace shifter {
namespace {

struct shift_res {
    auto operator<=>(shift_res const&) const = default;
    u32 shifted_data;
    bool carryout;
};
template<s_bit S>
using shift_t = std::conditional_t<S == s_bit::on, shift_res, u32>;
constexpr auto asr(u32 data, u32 amount) noexcept 
    -> u32 { return std::bit_cast<u32>(std::bit_cast<i32>(data) >> amount); }
template<s_bit S>
constexpr auto shift(u32 operand, [[maybe_unused]] register_manager const& rm) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on){ 
        return {operand, rm.cpsr().check_ccf(ccf::c)}; 
    } else {
        return operand; 
    }
}
template<s_bit S>
constexpr auto shiftlsr32([[maybe_unused]] u32 operand) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        return {0, static_cast<bool>(operand & (1_u32 << 31))};
    } else {
        return 0;
    } 
}
template<s_bit S>
constexpr auto shiftasr32(u32 operand) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        return {asr(operand, 31), static_cast<bool>(operand & (1_u32 << 31))};
    } else {
        return asr(operand, 31); 
    }
}
template<s_bit S>
constexpr auto shiftrrx(u32 operand, register_manager const& rm) noexcept 
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        return {(operand >> 1) | (static_cast<u32>(rm.cpsr().check_ccf(ccf::c)) << 31), static_cast<bool>(operand & 1)};
    } else {
        return (operand >> 1) | (static_cast<u32>(rm.cpsr().check_ccf(ccf::c)) << 31); 
    }
}
template<s_bit S>
constexpr auto shiftlsl(u32 shift_info, u32 operand) noexcept 
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        assert(shift_info != 0); 
        return {operand << shift_info, static_cast<bool>(operand & (1_u32 << (32 - shift_info)))};
    } else {
        assert(shift_info != 0);
        return operand << shift_info; 
    }
}
template<s_bit S>
constexpr auto shiftlsr(u32 shift_info, u32 operand) noexcept 
    -> shift_t<S> {
    if constexpr(S == s_bit::on) {
        assert(shift_info != 0); 
        return {operand >> shift_info, static_cast<bool>(operand & (1_u32 << (shift_info - 1)))}; 
    } else {
        assert(shift_info != 0);
        return operand >> shift_info; 
    }
}
template<s_bit S>
constexpr auto shiftasr(u32 shift_info, u32 operand) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        assert(shift_info != 0); 
        return asr(operand, shift_info); 
    } else {
        assert(shift_info != 0);
        return {asr(operand, shift_info), static_cast<bool>(operand & (1_u32 << (shift_info - 1)))};
    }
}
template<s_bit S>
constexpr auto shiftror(u32 shift_info, u32 operand) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::on) {
        assert(shift_info != 0);
        return {std::rotr(operand, shift_info), static_cast<bool>(operand & (1_u32 << (shift_info - 1)))};
    } else {
        assert(shift_info != 0);
        return std::rotr(operand, shift_info);
    }
}
template<s_bit S>
constexpr auto shiftrslsl(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        shift_res ret;
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount == 0) {
            ret.shifted_data = operand;
            ret.carryout = rm.cpsr().check_ccf(ccf::c);
        } else if (shift_amount > 32) {
            ret.shifted_data = 0;
            ret.carryout = 0;
        } else if (shift_amount == 32) {
            ret.shifted_data = 0;
            ret.carryout = static_cast<bool>(operand & 1);
        } else if (shift_amount < 32) {
            ret.shifted_data = operand << shift_amount;
            ret.carryout = static_cast<bool>(operand & (1_u32 << (32 - shift_amount)));
        }
        return ret; 
    } else {
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount >= 32) return 0;
        return operand << shift_amount; 
    }
}
template<s_bit S>
constexpr auto shiftrslsr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::on) {
        shift_res ret;
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount == 0) {
            ret.shifted_data = operand;
            ret.carryout = rm.cpsr().check_ccf(ccf::c);
        } else if (shift_amount > 32) {
            ret.shifted_data = 0;
            ret.carryout = 0;
        } else if (shift_amount == 32) {
            ret.shifted_data = 0;
            ret.carryout = static_cast<bool>(operand & (1_u32 << 31));
        } else if (shift_amount < 32) {
            ret.shifted_data = operand >> shift_amount;
            ret.carryout = static_cast<bool>(operand & (1_u32 << (shift_amount - 1)));
        }
        return ret; 
    } else {
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount >= 32) return 0;
        return operand >> shift_amount; 
    }
}
template<s_bit S>
constexpr auto shiftrsasr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount >= 32) return asr(operand, 31);
        return asr(operand, shift_amount); 
    } else  {
        shift_res ret;
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount == 0) {
            ret.shifted_data = operand;
            ret.carryout = rm.cpsr().check_ccf(ccf::c);
        } else if (shift_amount >= 32) {
            ret.shifted_data = asr(operand, 31);
            ret.carryout = static_cast<bool>(ret.shifted_data);
        } else if (shift_amount < 32) {
            ret.shifted_data = asr(operand, shift_amount);
            ret.carryout = static_cast<bool>(operand & (1_u32 << (shift_amount - 1)));
        }
        return ret; 
    }
}
template<s_bit S>
constexpr auto shiftrsror(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        auto const shift_amount = (rm[shift_info >> 1] & 0xff);
        return std::rotr(operand, shift_amount);
    } else {
        shift_res ret;
        auto const shift_amount = rm[shift_info >> 1] & 0xff;
        if (shift_amount == 0) {
            ret.shifted_data = operand;
            ret.carryout = rm.cpsr().check_ccf(ccf::c);
        } else if (shift_amount < 32) {
            ret.shifted_data = (operand >> shift_amount) | (operand << (32 - shift_amount));
            ret.carryout = static_cast<bool>(operand & (1_u32 << (shift_amount - 1)));
        } else if (shift_amount >= 32) {
            auto const adjusted_shift = shift_amount % 32;
            if (adjusted_shift == 0) {
                ret.shifted_data = operand;
                ret.carryout = static_cast<bool>(operand & (1_u32 << 31));
            } else {
                ret.shifted_data = (operand >> shift_amount) | (operand << (32 - shift_amount));
                ret.carryout = static_cast<bool>(operand & (1_u32 << (shift_amount - 1)));
            }
        }
    
        return ret;
    }
}
template<auto>
concept always_false = false;
template<shifts Shift, s_bit S>
[[nodiscard]] FGBA_FORCE_INLINE
constexpr auto barrel([[maybe_unused]] u32 shift_info, u32 operand, [[maybe_unused]] cpu::register_manager const& rm) noexcept
    -> shift_t<S> {

    if constexpr (Shift == cpu::shifts::null) {
        return shifter::shift<S>(operand, rm);
    } else if constexpr (Shift == shifts::asr32) {
        return shifter::shiftasr32<S>(operand);
    } else if constexpr (Shift == shifts::lsr32) {
        return shifter::shiftlsr32<S>(operand);
    } else if constexpr (Shift == shifts::rrx) {
        return shifter::shiftrrx<S>(operand, rm);
    } else if constexpr (Shift == shifts::lsl) {
        return shifter::shiftlsl<S>(shift_info, operand);
    } else if constexpr (Shift == shifts::lsr) {
        return shifter::shiftlsr<S>(shift_info, operand);
    } else if constexpr (Shift == shifts::asr) {
        return shifter::shiftasr<S>(shift_info, operand);
    } else if constexpr (Shift == shifts::ror) {
        return shifter::shiftror<S>(shift_info, operand);
    } else if constexpr (Shift == shifts::rslsl) {
        return shifter::shiftrslsl<S>(shift_info, operand, rm);
    } else if constexpr (Shift == shifts::rslsr) {
        return shifter::shiftrslsr<S>(shift_info, operand, rm);
    } else if constexpr (Shift == shifts::rsasr) {
        return shifter::shiftrsasr<S>(shift_info, operand, rm);
    } else if constexpr (Shift == shifts::rsror) {
        return shifter::shiftrsror<S>(shift_info, operand, rm);
    } else {
        static_assert(always_false<Shift>, "you passed UNACCEPTABLE shift enum");
    }

} 

}
}
}



#endif
