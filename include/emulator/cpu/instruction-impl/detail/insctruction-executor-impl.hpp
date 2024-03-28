#ifndef INSTRUCTION_EXECUTION_IMPLEMENTATION_HPP_CJNFJCFNK
#define INSTRUCTION_EXECUTION_IMPLEMENTATION_HPP_CJNFJCFNK
#include "emulator/cpu/instruction-impl/detail/instruction-executor-decl.hpp"
#include "fgba-defines.hpp"
#include "emulator/cpu/lla.hpp"
#include "emulator/cpu/shifter.hpp"

namespace fgba::cpu {

namespace  {

[[nodiscard]]constexpr auto andnot(u32 operand1, u32 operand2) { return operand1 & (~operand2); }
[[nodiscard]]constexpr auto andwastaken(u32 operand1, u32 operand2) { return operand1 & operand2; }
[[nodiscard]]constexpr auto eorwastaken(u32 operand1, u32 operand2) { return operand1 ^ operand2; }
[[nodiscard]]constexpr auto orrwastaken(u32 operand1, u32 operand2) { return operand1 | operand2; }
[[nodiscard]]constexpr auto notwastaken(u32 operand1) { return ~operand1; }
[[nodiscard]]constexpr auto identity(u32 operand1) { return operand1; }
[[nodiscard]]constexpr auto check_overflow(u32 operand1, u32 operand2, u32 result) {
    return ((operand1 >> 31l) == (operand2 >> 31u)) && ((operand1 >> 31u) != (result >> 31u));
}

}
//TODO: You will need to write a shit ton of tests for this shit
#define GEN_ARM_DATA_PROCESSING \
    GEN_ARM_DATA_PROCESSING_LOGICAL\
    GEN_ARM_DATA_PROCESSING_SINGLE_OPERAND\
    GEN_ARM_DATA_PROCESSING_ARITHMETIC
#define GEN_ARM_DATA_PROCESSING_ARITHMETIC\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, add, add_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, sub, sub_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, rsb, rsb_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, adc, adc_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, sbc, sbc_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION, rsc, rsc_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION_NORESULT, cmp, sub_impl)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_ARITHMETIC_OPERATION_NORESULT, cmn, add_impl)
#define GEN_ARM_DATA_PROCESSING_SINGLE_OPERAND \
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_SINGLE_OPERAND_OPERATION, mov,  identity) \
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_SINGLE_OPERAND_OPERATION, mvn, notwastaken)
#define GEN_ARM_DATA_PROCESSING_LOGICAL \
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION, and_, andwastaken)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION, eor, eorwastaken)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION, orr, orrwastaken)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION, bic, andnot)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION_NORESULT, tst, andwastaken)\
    GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(GEN_ARM_LOGICAL_OPERATION_NORESULT, teq, eorwastaken)
#define GEN_ARM_DATA_PROCESSING_SHIFT_AND_I_VARIATIONS(macro, arm_instruction_base, operator)\
    GEN_ARM_DATA_PROCESSING_SHIFT_VARIATIONS(macro##_WITH_SHIFT, arm_instruction_base, operator)\
    GEN_ARM_DATA_PROCESSING_IMMEDIATE_OPERAND(macro##_WITH_IMMEDIATE_OPERAND, arm_instruction_base, operator)
#define GEN_ARM_DATA_PROCESSING_IMMEDIATE_OPERAND(macro, arm_instruction_base, operator)\
    macro(arm_instruction_base, operator)
#define GEN_ARM_DATA_PROCESSING_SHIFT_VARIATIONS(macro, arm_instruction_base, operator)\
    macro(arm_instruction_base, operator,      )\
    macro(arm_instruction_base, operator, lsr32)\
    macro(arm_instruction_base, operator, asr32)\
    macro(arm_instruction_base, operator, rrx  )\
    macro(arm_instruction_base, operator, lsl  )\
    macro(arm_instruction_base, operator, lsr  )\
    macro(arm_instruction_base, operator, asr  )\
    macro(arm_instruction_base, operator, ror  )\
    macro(arm_instruction_base, operator, rslsl)\
    macro(arm_instruction_base, operator, rslsr)\
    macro(arm_instruction_base, operator, rsasr)\
    macro(arm_instruction_base, operator, rsror)

//
/// LOGICAL OPERATIONS CODE GENERATION (and, orr, eor, bic, tst, teq)
//
#define GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT(arm_instruction_base, operator, shift_type)\
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##shift_type(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 7) & 0b11111;\
    if constexpr (s) {\
        auto const [operand2, carryout] = shifter::shift##shift_type##_s(shift_info, cpu.m_registers[rm], cpu.m_registers);\
        auto res = cpu.m_registers[rd] = (operator)(cpu.m_registers[rn], operand2);\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
    } else {\
        auto const operand2 = shifter::shift##shift_type(shift_info, cpu.m_registers[rm], cpu.m_registers);\
        cpu.m_registers[rd] = (operator)(cpu.m_registers[rn], operand2);\
    }\
}
#define GEN_ARM_LOGICAL_OPERATION_WITH_IMMEDIATE_OPERAND(arm_instruction_base, operator)\
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##i(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    [[maybe_unused]]auto const shift_info = 2 * ((instruction >> 8) & 0b1111);\
    if constexpr (s) {\
        auto const [operand2, carryout] = shifter::shift##ror##_s(shift_info, instruction & 0xff, cpu.m_registers);\
        auto const res = cpu.m_registers[rd] = (operator)(cpu.m_registers[rn], operand2);\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
    } else {\
        auto const operand2 = shifter::shift##ror(shift_info, instruction & 0xff, cpu.m_registers);\
        cpu.m_registers[rd] = (operator)(cpu.m_registers[rn], operand2);\
    }\
}
#define GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, shift_type)\
inline auto instruction_executor::arm_##arm_instruction_base##shift_type(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 7) & 0b11111;\
    auto const [operand2, carryout] = shifter::shift##shift_type##_s(shift_info, cpu.m_registers[rm], cpu.m_registers);\
    auto const res = (operator)(cpu.m_registers[rn], operand2);\
    cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
    cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
    cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
}
#define GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_IMMEDIATE_OPERAND(arm_instruction_base, operator)\
inline auto instruction_executor::arm_##arm_instruction_base##i(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rn = (instruction >> 16) & 0xf;\
    [[maybe_unused]]auto const shift_info = 2 * ((instruction >> 8) & 0b1111);\
    auto const [operand2, carryout] = shifter::shift##ror##_s(shift_info, instruction & 0xff, cpu.m_registers);\
    auto const res = (operator)(cpu.m_registers[rn], operand2);\
    cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
    cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
    cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
}

//
/// SINGLE OPERAND CODE GENERATION (mov, mvn)
//
#define GEN_ARM_SINGLE_OPERAND_OPERATION_WITH_SHIFT(arm_instruction_base, operator, shift_type)\
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##shift_type(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    [[maybe_unused]]auto const shift_info = (instruction >> 7) & 0b11111;\
    if constexpr (s) {\
        auto const [operand2, carryout] = shifter::shift##shift_type##_s(shift_info, cpu.m_registers[rm], cpu.m_registers);\
        auto const res = cpu.m_registers[rd] = (operator)(operand2);\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
    } else {\
        auto const operand2 = shifter::shift##shift_type(shift_info, cpu.m_registers[rm], cpu.m_registers);\
        cpu.m_registers[rd] = (operator)(operand2);\
    }\
}
#define GEN_ARM_SINGLE_OPERAND_OPERATION_WITH_IMMEDIATE_OPERAND(arm_instruction_base, operator)\
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##i(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    [[maybe_unused]]auto const shift_info = 2 * ((instruction >> 8) & 0b1111);\
    if constexpr (s) {\
        auto const [operand2, carryout] = shifter::shiftror_s(shift_info, instruction & 0xff, cpu.m_registers);\
        auto const res = cpu.m_registers[rd] = (operator)(operand2);\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not res);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));\
    } else {\
        auto const operand2 = shifter::shiftror(shift_info, instruction & 0xff, cpu.m_registers);\
        cpu.m_registers[rd] = (operator)(operand2);\
    }\
}
//
/// ARITHMETIC OPERATIONS CODE GENERATION (add, adc, sub, sbc, rsb, rsc, cmp, cmn)
//
#define GEN_ARM_ARITHMETIC_OPERATION_WITH_SHIFT(arm_instruction_base, operator, shift_type) \
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##shift_type(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    auto& destination = cpu.m_registers[rd];\
    auto const shift_info = (instruction >> 7) & 0b11111;\
    auto const operand2 = shifter::shift##shift_type(shift_info, cpu.m_registers[rm], cpu.m_registers);\
    [[maybe_unused]]auto const carryout = (operator)(cpu.m_registers[rn], operand2, &destination, cpu.m_registers.cpsr().check_ccf(ccf::c));\
    if constexpr (s) {\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not destination);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));\
        cpu.m_registers.cpsr().set_ccf(ccf::v, check_overflow(cpu.m_registers[rn], operand2, destination));\
    }\
}
#define GEN_ARM_ARITHMETIC_OPERATION_WITH_IMMEDIATE_OPERAND(arm_instruction_base, operator) \
template<bool s>\
auto instruction_executor::arm_##arm_instruction_base##i(arm7tdmi& cpu, u32 instruction) -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto& destination = cpu.m_registers[rd];\
    [[maybe_unused]]auto const shift_info = 2 * ((instruction >> 8) & 0b1111);\
    auto const operand2 = shifter::shiftror(shift_info, instruction & 0xff, cpu.m_registers);\
    [[maybe_unused]]auto const carryout = (operator)(cpu.m_registers[rn], operand2, &destination, cpu.m_registers.cpsr().check_ccf(ccf::c));\
    if constexpr (s) {\
        cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
        cpu.m_registers.cpsr().set_ccf(ccf::z, not destination);\
        cpu.m_registers.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));\
        cpu.m_registers.cpsr().set_ccf(ccf::v, check_overflow(cpu.m_registers[rn], operand2, destination));\
    }\
}
#define GEN_ARM_ARITHMETIC_OPERATION_NORESULT_WITH_SHIFT(arm_instruction_base, operator, shift_type) \
inline auto instruction_executor::arm_##arm_instruction_base##shift_type(arm7tdmi& cpu, u32 instruction) \
    -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto const rm = (instruction >>  0) & 0xf;\
    auto& destination = cpu.m_registers[rd];\
    [[maybe_unused]]auto const shift_info = (instruction >> 7) & 0b11111;\
    auto const operand2 = shifter::shift##shift_type(shift_info, cpu.m_registers[rm], cpu.m_registers);\
    auto const carryout = (operator)(cpu.m_registers[rn], operand2, &destination, cpu.m_registers.cpsr().check_ccf(ccf::c));\
    cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
    cpu.m_registers.cpsr().set_ccf(ccf::z, not destination);\
    cpu.m_registers.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));\
    cpu.m_registers.cpsr().set_ccf(ccf::v, check_overflow(cpu.m_registers[rn], operand2, destination));\
}
#define GEN_ARM_ARITHMETIC_OPERATION_NORESULT_WITH_IMMEDIATE_OPERAND(arm_instruction_base, operator) \
inline auto instruction_executor::arm_##arm_instruction_base##i(arm7tdmi& cpu, u32 instruction) \
    -> void {\
    auto const rd = (instruction >> 12) & 0xf;\
    auto const rn = (instruction >> 16) & 0xf;\
    auto& destination = cpu.m_registers[rd];\
    [[maybe_unused]]auto const shift_info = 2 * ((instruction >> 8) & 0b1111);\
    auto const operand2 = shifter::shiftror(shift_info, instruction & 0xff, cpu.m_registers);\
    auto const carryout = (operator)(cpu.m_registers[rn], operand2, &destination, cpu.m_registers.cpsr().check_ccf(ccf::c));\
    cpu.m_registers.cpsr().set_ccf(ccf::c, carryout);\
    cpu.m_registers.cpsr().set_ccf(ccf::z, not destination);\
    cpu.m_registers.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));\
    cpu.m_registers.cpsr().set_ccf(ccf::v, check_overflow(cpu.m_registers[rn], operand2, destination));\
}
GEN_ARM_DATA_PROCESSING

#undef GEN_ARM_DATA_PROCESSING
#undef GEN_ARM_DATA_PROCESSING_LOGICAL
#undef GEN_ARM_DATA_PROCESSING_SINGLE_OPERAND
#undef GEN_ARM_DATA_PROCESSING_SHIFT_VARIATIONS
#undef GEN_ARM_LOGICAL_OPERATION_WITH_SHIFT
#undef GEN_ARM_LOGICAL_OPERATION_NORESULT_WITH_SHIFT
#undef GEN_ARM_SINGLE_OPERAND_OPERATION_WITH_SHIFT

}


#endif