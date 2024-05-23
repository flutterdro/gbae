#ifndef INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#define INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpu/opcodes.hpp"

namespace fgba::cpu {
auto execute_arm(arm7tdmi&, arm_instruction, u32) -> void;
} // namespace fgba::cpu

#endif
