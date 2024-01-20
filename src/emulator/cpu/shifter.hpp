#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <cstdint>
#include <bit>
#include "cpudefines.hpp"
#include "registermanager.hpp"
namespace fgba::cpu {
namespace shifter {
namespace {

struct shift_res {
    u32 shifted_data;
    bool carryout;
};
constexpr auto asr(u32 data, u32 amount) noexcept 
    -> u32 { return std::bit_cast<u32>(std::bit_cast<i32>(data) >> amount); }
constexpr auto shift(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return operand; }
constexpr auto shiftlsr32(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return 0; }
constexpr auto shiftasr32(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return asr(operand, 31); }
constexpr auto shiftrrx(u32, u32 operand, register_manager const& rm) noexcept 
    -> u32 { return (operand >> 1) | (rm.cpsr().check_ccf(ccf::c) << 31); }
constexpr auto shiftlsl(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> u32 { assert(shift_info != 0); return operand << shift_info; }
constexpr auto shiftlsr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { assert(shift_info != 0); return operand >> shift_info; }
constexpr auto shiftasr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { assert(shift_info != 0); return asr(operand, shift_info); }
constexpr auto shiftror(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { assert(shift_info != 0); return (operand >> shift_info) | (operand << (32 - shift_info)); }
constexpr auto shiftrslsl(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { 
    // I actually have no clue why I use auto. u32 - 3 symbols, auto - 4 symbols. me dumdum
    auto const shift_amount = rm[shift_info >> 1] & 0xff;
    if (shift_amount >= 32) return 0;
    return operand << shift_amount; 
}
constexpr auto shiftrslsr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { 
    auto const shift_amount = rm[shift_info >> 1] & 0xff;
    if (shift_amount >= 32) return 0;
    return operand >> shift_amount; 
}
constexpr auto shiftrsasr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { 
    auto const shift_amount = rm[shift_info >> 1] & 0xff;
    if (shift_amount >= 32) return asr(operand, 31);
    return asr(operand, shift_amount); 
}
constexpr auto shiftrsror(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { 
    auto const shift_amount = (rm[shift_info >> 1] & 0xff) % 32;
    if (shift_amount == 0) return operand;
    return (operand >> shift_amount) | (operand << (32 - shift_amount));
}
constexpr auto shift_s(u32, u32 operand, register_manager const& rm) noexcept 
    -> shift_res { return {operand, rm.cpsr().check_ccf(ccf::c)}; }
constexpr auto shiftlsr32_s(u32, u32 operand, register_manager const&) noexcept 
    -> shift_res { return {0, static_cast<bool>(operand & (1_u32 << 31))}; }
constexpr auto shiftasr32_s(u32, u32 operand, register_manager const&) noexcept 
    -> shift_res { return {asr(operand, 31), static_cast<bool>(operand & (1_u32 << 31))}; }
constexpr auto shiftrrx_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_res { return {(operand >> 1) | (rm.cpsr().check_ccf(ccf::c) << 31), static_cast<bool>(operand & 1)}; }
constexpr auto shiftlsl_s(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> shift_res { assert(shift_info != 0); return {operand << shift_info, static_cast<bool>(operand & (1_u32 << (32 - shift_info)))}; }
constexpr auto shiftlsr_s(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> shift_res { assert(shift_info != 0); return {operand >> shift_info, static_cast<bool>(operand & (1_u32 << (shift_info - 1)))}; }
constexpr auto shiftasr_s(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> shift_res { assert(shift_info != 0); return {asr(operand, shift_info), static_cast<bool>(operand & (1_u32 << (shift_info - 1)))}; }
constexpr auto shiftror_s(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> shift_res { assert(shift_info != 0); return {(operand >> shift_info) | (operand << (32 - shift_info)), static_cast<bool>(operand & (1_u32 << (shift_info - 1)))}; }
constexpr auto shiftrslsl_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_res { 
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
}
constexpr auto shiftrslsr_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_res {
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
}
constexpr auto shiftrsasr_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_res { 
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
constexpr auto shiftrsror_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> shift_res { 
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
}
}



#endif