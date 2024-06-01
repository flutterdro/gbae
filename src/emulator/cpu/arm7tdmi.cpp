#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpudefines.hpp"
#include "emulator/cpu/instruction-impl/implementation.hpp"
#include "emulator/cpu/opcodes.hpp"
#include <concepts>
#include <utility>
namespace fgba::cpu {

arm7tdmi::arm7tdmi() {}

auto arm7tdmi::advance_execution() -> void {
    prefetch();
    auto nextisntr = m_prefetch_buffer.front();
    m_prefetch_buffer.pop();
    if (m_registers.cpsr().is_thumb()) {
        execute_thumb(nextisntr); 
    } else {
        execute_arm(nextisntr);
    }
}

auto arm7tdmi::execute_arm(u32 const instruction) -> void {
 //   arm_instruction_set candidate = bx; 
 //   for (;candidate < undefined; candidate = static_cast<arm_instruction_set>(candidate + 1)) {      
 //       if ((instruction & opcodes[candidate].mask) == opcodes[candidate].opcode) {
 //           break;
 //       }
 //   }
 //   fgba::cpu::execute_arm(*this, candidate, instruction);
}

auto arm7tdmi::execute_thumb(u16)
    -> void {}

auto arm7tdmi::prefetch() -> void {
}
auto arm7tdmi::flush() -> void {
    while (not m_prefetch_buffer.empty()) {
        m_prefetch_buffer.pop();
    }
}
}
namespace fgba::cpu {
}

namespace fgba::cpu {



}
