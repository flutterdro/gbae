#ifndef INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#define INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#include "../arm7tdmi.hpp"

namespace fgba::cpu {
auto execute_arm(arm7tdmi&, arm_instruction_set, u32) -> void;
}

#endif