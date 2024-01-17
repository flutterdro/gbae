#include "arm7tdmi.hpp"
#include "cpudefines.hpp"
#include "shifter.hpp"
#include "lla.hpp"
#include <__utility/to_underlying.h>
#include <utility>
namespace fgba::cpu {
std::array<u32, std::to_underlying(arm_instruction_set::undefined)> arm_instruction_mask{};
std::array<u32, std::to_underlying(arm_instruction_set::undefined)> arm_instruction_opc{};

arm7tdmi::arm7tdmi()
    : m_arm_impl(std::to_underlying(arm_instruction_set::undefined)) {
    m_arm_impl[arm_instruction_set::bx]     = &arm7tdmi::arm_bx;
    m_arm_impl[arm_instruction_set::b]      = &arm7tdmi::arm_b;  
    m_arm_impl[arm_instruction_set::bl]     = &arm7tdmi::arm_bl; 
}

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
        if ((instruction & arm_instruction_mask[candidate]) == arm_instruction_opc[candidate]) {
            break;
        }
    }
    (*this.*(m_arm_impl[candidate]))(instruction);
}

auto arm7tdmi::prefetch() -> void {
    bool is_thumb = m_registers.cpsr().is_thumb();
    bus::signals const signal{
        .address{m_registers.pc()},
        .mas{2u - is_thumb},
        .nopc{0},
        .tbit{is_thumb}
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
auto arm7tdmi::arm_bx(u32 const instruction) 
    -> void {
    u32 const register_val = m_registers[instruction & 0xf];
    flush();
    m_registers.pc() = register_val & ~1_u32;
    m_registers.cpsr().use_thumb(register_val & 1_u32);
    prefetch();
    prefetch();
}
auto arm7tdmi::arm_b(u32 const instruction)
    -> void {
    flush();
    m_registers.pc() += signextend<i32, 24>((instruction & 0xffffff) << 2) - 8;
    prefetch();
    prefetch();
}
auto arm7tdmi::arm_bl(u32 const instruction)
    -> void {
    m_registers[14] = m_registers.pc() - 4;
    flush();
    m_registers.pc() += signextend<i32, 24>((instruction & 0xffffff) << 2) - 8;
    prefetch();
    prefetch();
}
[[nodiscard, gnu::always_inline]]constexpr auto andnot(u32 operand1, u32 operand2) { return operand1 & (~operand2); }
[[nodiscard, gnu::always_inline]]constexpr auto andwastaken(u32 operand1, u32 operand2) { return operand1 & (~operand2); }
[[nodiscard, gnu::always_inline]]constexpr auto eorwastaken(u32 operand1, u32 operand2) { return operand1 & (~operand2); }
[[nodiscard, gnu::always_inline]]constexpr auto orrwastaken(u32 operand1, u32 operand2) { return operand1 & (~operand2); }
//TODO: You will need to write a shit ton of tests for this shit
#define GEN_ARM_DATA_PROCESSING \
    GEN_ARM_DATA_PROCESSING_LOGICAL
#define GEN_ARM_DATA_PROCESSING_LOGICAL \
    GEN_ARM_LOGICAL_OPERATION_GROUP         (and_, andwastaken)\
    GEN_ARM_LOGICAL_OPERATION_GROUP         (eor,  eorwastaken)\
    GEN_ARM_LOGICAL_OPERATION_GROUP         (orr,  orrwastaken)\
    GEN_ARM_LOGICAL_OPERATION_GROUP         (bic,  andnot   )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_GROUP(tst,  andwastaken)\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_GROUP(teq,  eorwastaken)
#define GEN_ARM_LOGICAL_OPERATION_GROUP(arm_instruction_base, operator) \
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator,      )\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, lsr32)\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, asr32)\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, rrx  )\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, lsl  )\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, lsr  )\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, asr  )\
    GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, ror  )
#define GEN_ARM_LOGICAL_OPERATION_NORESULT_GROUP(arm_instruction_base, operator) \
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator,      )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, lsr32)\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, asr32)\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, rrx  )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, lsl  )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, lsr  )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, asr  )\
    GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, ror  )
#define GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, shift_type)\
auto arm7tdmi::arm_##arm_instruction_base##shift_type(u32 const instruction) noexcept -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 4) & 0xfff;\
    auto const operand2 = shifter::shift##shift_type(shift_info, m_registers[rm], m_registers.cpsr().check_ccf(ccf::c));\
    m_registers[rd] = operator(m_registers[rn], operand2);\
}\
auto arm7tdmi::arm_##arm_instruction_base##s##shift_type(u32 const instruction) noexcept \
    -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 4) & 0xfff;\
    auto const [operand2, carryout] = shifter::shift##shift_type##_s(shift_info, m_registers[rm], m_registers.cpsr().check_ccf(ccf::c));\
    auto const res = m_registers[rd] = operator(m_registers[rn], operand2);\
    m_registers.cpsr().set_ccf(ccf::c, carryout);\
    m_registers.cpsr().set_ccf(ccf::z, not res);\
    m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
}
#define GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, shift_type)\
auto arm7tdmi::arm_##arm_instruction_base##shift_type(u32 const instruction) noexcept -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 4) & 0xfff;\
    auto const operand2 = shifter::shift##shift_type(shift_info, m_registers[rm], m_registers.cpsr().check_ccf(ccf::c));\
    m_registers[rd] = operator(m_registers[rn], operand2);\
}\

GEN_ARM_DATA_PROCESSING

}