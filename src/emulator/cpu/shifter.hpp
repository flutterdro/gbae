#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <cstdint>
#include <bit>
#include "cpudefines.hpp"
#include "registermanager.hpp"
namespace fgba::cpu {
namespace shifter {

struct shift_res {
    u32 shifted_data;
    bool carryout;
};
constexpr auto asr(u32 data, u32 amount) noexcept 
    -> u32 { return static_cast<u32>(static_cast<i32>(data) >> amount); }
constexpr auto shift(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return operand; }
constexpr auto shiftlsr32(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return 0; }
constexpr auto shiftasr32(u32, u32 operand, register_manager const&) noexcept
    -> u32 { return asr(operand, 31); }
constexpr auto shiftrrx(u32, u32 operand, register_manager const& rm) noexcept 
    -> u32 { return (operand >> 1) | (rm.cpsr().check_ccf(ccf::c) << 31); }
constexpr auto shiftlsl(u32 shift_info, u32 operand, register_manager const&) noexcept 
    -> u32 { return operand << shift_info; }
constexpr auto shiftlsr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { return operand >> shift_info; }
constexpr auto shiftasr(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { return asr(operand, shift_info); }
constexpr auto shiftror(u32 shift_info, u32 operand, register_manager const& rm) noexcept 
    -> u32 { return (operand >> shift_info) | (operand << (32 - shift_info)); }
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
    if (shift_amount >= 32) return asr(operand, 31);
    return asr(operand, shift_amount); 
}
constexpr auto shift_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftlsr32_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftasr32_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftrrx_s  (u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftlsl_s  (u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftlsr_s  (u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftasr_s  (u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftror_s  (u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftrslsl_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftrslsr_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftrsasr_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
constexpr auto shiftrsror_s(u32 shift_info, u32 operand, register_manager const& rm) noexcept -> shift_res { return {0, 0}; }
}
}



#endif