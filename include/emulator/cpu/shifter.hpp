#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <bit>
#include <type_traits>
#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
#include "registermanager.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
namespace fgba::cpu {
namespace shifter {
namespace {

struct shift_res {
    auto operator<=>(shift_res const&) const noexcept = default;
    word shifted_data;
    bit carryout;
};
template<s_bit S>
using shift_t = std::conditional_t<S == s_bit::on, shift_res, word>;
constexpr auto asr(u32 data, u32 amount) noexcept 
    -> u32 { return std::bit_cast<u32>(std::bit_cast<i32>(data) >> amount); }
template<s_bit S>
constexpr auto shift(word const operand, bit const carryin) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on){ 
        return {operand, carryin}; 
    } else {
        return operand; 
    }
}
template<s_bit S>
constexpr auto shiftlsr32([[maybe_unused]] word const operand) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        return {0_word, operand[31]};
    } else {
        return 0_word;
    } 
}
template<s_bit S>
constexpr auto shiftasr32(word const operand) noexcept
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        return {operand.asr(31), operand[31]};
    } else {
        return operand.asr(31); 
    }
}
template<s_bit S>
constexpr auto shiftrrx(word const operand, bit const carryin) noexcept 
    -> shift_t<S> { 
    word result = operand.lsr(1);
    result[31] = carryin;
    if constexpr (S == s_bit::on) {
        return {result, operand[0]};
    } else {
        return result; 
    }
}
template<s_bit S>
constexpr auto shiftlsl(u32 const shift_amount, word const operand) noexcept 
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        assert(shift_amount != 0_u32); 
        return {operand.lsl(shift_amount), operand[32 - shift_amount]};
    } else {
        assert(shift_amount != 0_u32);
        return operand.lsl(shift_amount); 
    }
}
template<s_bit S>
constexpr auto shiftlsr(u32 const shift_amount, word const operand) noexcept 
    -> shift_t<S> {
    if constexpr(S == s_bit::on) {
        assert(shift_amount != 0_u32); 
        return {operand.lsr(shift_amount), operand[shift_amount - 1]}; 
    } else {
        assert(shift_amount != 0_u32);
        return operand.lsr(shift_amount); 
    }
}
template<s_bit S>
constexpr auto shiftasr(u32 const shift_amount, word const operand) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        assert(shift_amount != 0); 
        return operand.asr(shift_amount); 
    } else {
        assert(shift_amount != 0);
        return {operand.asr(shift_amount), operand[shift_amount - 1]};
    }
}
template<s_bit S>
constexpr auto shiftror(u32 const shift_amount, word const operand) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::on) {
        assert(shift_amount != 0);
        return {operand.ror(shift_amount), operand[shift_amount - 1]};
    } else {
        assert(shift_amount != 0);
        return operand.ror(shift_amount);
    }
}
template<s_bit S>
constexpr auto shiftrslsl(u32 const shift_amount, word const operand, bit const carryin) noexcept 
    -> shift_t<S> { 
    if constexpr (S == s_bit::on) {
        shift_res ret;
        if (shift_amount == 0_u32) {
            ret.shifted_data = operand;
            ret.carryout = carryin;
        } else if (shift_amount > 32) {
            ret.shifted_data = 0_word;
            ret.carryout = 0_bit;
        } else if (shift_amount == 32_u32) {
            ret.shifted_data = 0_word;
            ret.carryout = operand[0];
        } else if (shift_amount < 32_u32) {
            ret.shifted_data = operand.lsl(shift_amount);
            ret.carryout = operand[32 - shift_amount];
        }
        return ret; 
    } else {
        if (shift_amount >= 32) return 0_word;
        return operand.lsl(shift_amount); 
    }
}
template<s_bit S>
constexpr auto shiftrslsr(u32 const shift_amount, word const operand, bit const carryin) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::on) {
        shift_res ret;
        if (shift_amount == 0_u32) {
            ret.shifted_data = operand;
            ret.carryout = carryin;
        } else if (shift_amount > 32_u32) {
            ret.shifted_data = 0_word;
            ret.carryout = 0_bit;
        } else if (shift_amount == 32_u32) {
            ret.shifted_data = 0_word;
            ret.carryout = operand[31];
        } else if (shift_amount < 32) {
            ret.shifted_data = operand.lsr(shift_amount);
            ret.carryout = operand[shift_amount - 1];
        }
        return ret; 
    } else {
        if (shift_amount >= 32) return 0_word;
        return operand.lsr(shift_amount); 
    }
}
template<s_bit S>
constexpr auto shiftrsasr(u32 const shift_amount , word const operand, bit const carryin) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        if (shift_amount >= 32) return operand.asr(31);
        return operand.asr(shift_amount); 
    } else  {
        shift_res ret;
        if (shift_amount == 0_u32) {
            ret.shifted_data = operand;
            ret.carryout = carryin;
        } else if (shift_amount >= 32_u32) {
            ret.shifted_data = operand.asr(31);
            ret.carryout = ret.shifted_data[0];
        } else if (shift_amount < 32) {
            ret.shifted_data = operand.asr(shift_amount);
            ret.carryout = operand[shift_amount - 1];
        }
        return ret; 
    }
}
template<s_bit S>
constexpr auto shiftrsror(u32 const shift_amount, word const operand, bit const carryin) noexcept 
    -> shift_t<S> {
    if constexpr (S == s_bit::off) {
        return operand.ror(shift_amount);
    } else {
        shift_res ret;
        if (shift_amount == 0) {
            ret.shifted_data = operand;
            ret.carryout = carryin;
        } else if (shift_amount < 32) {
            ret.shifted_data = operand.ror(shift_amount);
            ret.carryout = operand[shift_amount - 1];
        } else if (shift_amount >= 32) {
            auto const adjusted_shift = shift_amount % 32;
            if (adjusted_shift == 0) {
                ret.shifted_data = operand;
                ret.carryout = operand[31];
            } else {
                ret.shifted_data = operand.ror(adjusted_shift);
                ret.carryout = operand[adjusted_shift - 1];
            }
        }
    
        return ret;
    }
}
template<auto>
concept always_false = false;
template<shifts Shift, s_bit S>
[[nodiscard]] FGBA_FORCE_INLINE
constexpr auto barrel([[maybe_unused]] word const instruction, word const operand, [[maybe_unused]] cpu::register_manager const& rm) noexcept
    -> shift_t<S> {

    [[maybe_unused]]auto const shift_amount          = instruction[11, 5].value;
    [[maybe_unused]]auto const shift_register        = instruction[11, 8].value;
    [[maybe_unused]]auto const register_shift_amount = rm[shift_register].value;
    [[maybe_unused]]auto const carryin               = rm.cpsr().check_ccf(ccf::c);


    if constexpr (Shift == cpu::shifts::null) {
        return shifter::shift<S>(operand, carryin);
    } else if constexpr (Shift == shifts::asr32) {
        return shifter::shiftasr32<S>(operand);
    } else if constexpr (Shift == shifts::lsr32) {
        return shifter::shiftlsr32<S>(operand);
    } else if constexpr (Shift == shifts::rrx) {
        return shifter::shiftrrx<S>(operand, carryin);
    } else if constexpr (Shift == shifts::lsl) {
        return shifter::shiftlsl<S>(shift_amount, operand);
    } else if constexpr (Shift == shifts::lsr) {
        return shifter::shiftlsr<S>(shift_amount, operand);
    } else if constexpr (Shift == shifts::asr) {
        return shifter::shiftasr<S>(shift_amount, operand);
    } else if constexpr (Shift == shifts::ror) {
        return shifter::shiftror<S>(shift_amount, operand);
    } else if constexpr (Shift == shifts::rslsl) {
        return shifter::shiftrslsl<S>(shift_amount, operand, carryin);
    } else if constexpr (Shift == shifts::rslsr) {
        return shifter::shiftrslsr<S>(shift_amount, operand, carryin);
    } else if constexpr (Shift == shifts::rsasr) {
        return shifter::shiftrsasr<S>(shift_amount, operand, carryin);
    } else if constexpr (Shift == shifts::rsror) {
        return shifter::shiftrsror<S>(shift_amount, operand, carryin);
    } else {
        static_assert(always_false<Shift>, "you passed UNACCEPTABLE shift enum");
    }

} 

}
}
}



#endif
