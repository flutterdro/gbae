#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <cstdint>
#include <bit>
#include "cpudefines.hpp"
namespace fgba::cpu {
namespace shifter {

struct shift_res {
    u32 shifted_data;
    bool carryout;
};

auto shift(u32 shift_info, u32 operand, bool carryin)      -> u32 {}
auto shiftlsr32(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shiftasr32(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shiftrrx(u32 shift_info, u32 operand, bool carryin)   -> u32 {}
auto shiftlsl(u32 shift_info, u32 operand, bool carryin)   -> u32 {}
auto shiftlsr(u32 shift_info, u32 operand, bool carryin)   -> u32 {}
auto shiftasr(u32 shift_info, u32 operand, bool carryin)   -> u32 {}
auto shiftror(u32 shift_info, u32 operand, bool carryin)   -> u32 {}
auto shiftrslsl(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shiftrslsr(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shiftrsasr(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shiftrsror(u32 shift_info, u32 operand, bool carryin) -> u32 {}
auto shift_s(u32 shift_info, u32 operand, bool carryin)      -> shift_res {}
auto shiftlsr32_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
auto shiftasr32_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
auto shiftrrx_s(u32 shift_info, u32 operand, bool carryin)   -> shift_res {}
auto shiftlsl_s(u32 shift_info, u32 operand, bool carryin)   -> shift_res {}
auto shiftlsr_s(u32 shift_info, u32 operand, bool carryin)   -> shift_res {}
auto shiftasr_s(u32 shift_info, u32 operand, bool carryin)   -> shift_res {}
auto shiftror_s(u32 shift_info, u32 operand, bool carryin)   -> shift_res {}
auto shiftrslsl_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
auto shiftrslsr_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
auto shiftrsasr_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
auto shiftrsror_s(u32 shift_info, u32 operand, bool carryin) -> shift_res {}
}
}



#endif