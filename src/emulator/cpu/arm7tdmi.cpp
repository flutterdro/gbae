#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpudefines.hpp"
#include "emulator/cpu/instruction-impl/implementation.hpp"
#include "emulator/cpu/opcodes.hpp"


namespace fgba::cpu {

arm7tdmi::arm7tdmi() {}

auto arm7tdmi::advance_execution() -> void {
    if (m_registers.cpsr().is_thumb()) {
        //TODO: implement thumb
    } else {
        auto current_instruction = m_prefetch_buffer.read<u32>();
        prefetch();
        auto decoded = cpu::decode_arm(current_instruction);
        cpu::execute_arm(*this, decoded, current_instruction);
    }
    increment_program_counter();
}

auto arm7tdmi::prefetch() -> void {
    if (m_registers.cpsr().is_thumb()) {
        //TODO: implement thumb
    } else {
        m_bus.access_read(address{m_registers.pc()}, data_size::word);
        auto bus_contents = m_bus.load_from();
        m_prefetch_buffer.write<u32>(bus_contents);
    }
}

auto arm7tdmi::flush_pipeline() -> void {
    m_prefetch_buffer.flush();
}

auto arm7tdmi::increment_program_counter() -> void {
    if (m_registers.cpsr().is_thumb()) {
        m_registers.pc() += 2_u32;
    } else {
        m_registers.pc() += 4_u32;
    }
}
auto arm7tdmi::refill_pipeline() -> void {
    if (m_registers.cpsr().is_thumb()) {
        prefetch();
        m_registers.pc() += 2;
        prefetch();
    } else {
        prefetch();
        m_registers.pc() += 4;
        prefetch();
    }
}

auto arm7tdmi::connect_bus(connector connector)
    -> void { m_bus.connect(connector); }

} // namespace fgba::cpu
