#ifndef INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#define INSTRUCTION_EXECUTION_HPP_FOCNOCFNR
#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpu/opcodes.hpp"

namespace fgba::cpu {
namespace arm {
auto execute(arm7tdmi&, instruction_spec, instruction) -> void;
}
} // namespace fgba::cpu

#endif
