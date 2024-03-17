#include "arm7tdmi.hpp"
#include "cpudefines.hpp"
#include "shifter.hpp"
#include "lla.hpp"
#include "instruction-implementation/implementation.hpp"
#include "opcodes.hpp"
#include <__concepts/invocable.h>
#include <__utility/to_underlying.h>
#include <array>
#include <functional>
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
    arm_instruction_set candidate = bx; 
    for (;candidate < undefined; candidate = static_cast<arm_instruction_set>(candidate + 1)) {      
        if ((instruction & opcodes[candidate].mask) == opcodes[candidate].opcode) {
            break;
        }
    }
    fgba::cpu::execute_arm(*this, candidate, instruction);
}

auto arm7tdmi::execute_thumb(u16)
    -> void {}

auto arm7tdmi::prefetch() -> void {
    bool is_thumb = m_registers.cpsr().is_thumb();
    bus::signals const signal{
        .address = m_registers.pc(),
        .mas     = 2u - is_thumb,
        .nopc    = 0,
        .tbit    = is_thumb
    };
    m_prefetch_buffer.push(m_bus.read(signal));
    m_registers.pc() += 4 - 2 * is_thumb;
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