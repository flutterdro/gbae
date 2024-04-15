#ifndef INSTRUCTION_EXECUTION_HPP_KCNFKJCN
#define INSTRUCTION_EXECUTION_HPP_KCNFKJCN

#include "emulator/cpudefines.hpp"
#include "emulator/cpu/arm7tdmi.hpp"
#include "fgba-defines.hpp"
#include "emulator/cpu/shifter.hpp"

namespace fgba::cpu {


using single_operand_operation = auto(*)(u32) -> u32;
using logical_operation        = auto(*)(u32, u32) -> u32;
using arithmetic_operation     = auto(*)(u32, u32, u32*, u32) -> bool;
struct instruction_executor {
    static auto arm_bx(arm7tdmi& cpu, u32 instruction) 
        -> void; 
    template<immediate_operand I, s_bit S, shifts Shift, arithmetic_operation Operation>
    static auto arm_arithmetic(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand I, shifts Shift, arithmetic_operation Operation>
    static auto arm_arithmetic(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand I, s_bit S, shifts Shift, logical_operation Operation>
    static auto arm_logical(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand I, shifts Shift, logical_operation Operation>
    static auto arm_logical(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand I, s_bit S, shifts Shift, single_operand_operation Operation>
    static auto arm_single_operand(arm7tdmi&, u32 instruction) 
        -> void;
};
} //namespace fgba::cpu

namespace fgba::cpu {

[[nodiscard]]constexpr auto check_overflow(u32 operand1, u32 operand2, u32 result) {
    return ((operand1 >> 31l) == (operand2 >> 31u)) && ((operand1 >> 31u) != (result >> 31u));
}

template<s_bit S>
constexpr auto get_operand2(auto const shifted) -> u32 {
    if constexpr (S == s_bit::on) return shifted.shifted_data;
    else return shifted;
}

template<immediate_operand I, s_bit S, shifts Shift>
FGBA_FORCE_INLINE
constexpr auto i_have_no_clue_how_to_name_this(arm7tdmi const& cpu, u32 instruction) {
    auto const& regs = cpu.m_registers;
    if constexpr (I == immediate_operand::on) {
        auto const shift_info = 2 * ((instruction >> 8) & 0b1111);
        return shifter::barrel<shifts::ror, S>(shift_info, instruction & 0xff, regs);
    } else {
        auto const rm = (instruction >>  0) & 0xf;
        auto const shift_info = (instruction >> 7) & 0b11111;
        return shifter::barrel<Shift, S>(shift_info, regs[rm], regs);
    }
}

template<immediate_operand I, s_bit S, shifts Shift, logical_operation Operation>
auto instruction_executor::arm_logical(arm7tdmi& cpu, u32 instruction) -> void {
    auto const rd = (instruction >> 12) & 0xf;
    auto const rn = (instruction >> 16) & 0xf;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, S, Shift>(cpu, instruction);
    auto const operand2 = cpu::get_operand2<S>(shifted_result);
    [[maybe_unused]]auto const res = regs[rd] = Operation(regs[rn], operand2); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
        regs.cpsr().set_ccf(ccf::z, not res);
        regs.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));
    }
}

template<immediate_operand I, shifts Shift, logical_operation Operation>
auto instruction_executor::arm_logical(arm7tdmi& cpu, u32 instruction) -> void {
    auto const rn = (instruction >> 16) & 0xf;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, s_bit::on, Shift>(cpu, instruction);
    auto const operand2 = cpu::get_operand2<s_bit::on>(shifted_result);
    auto const res = Operation(regs[rn], operand2); 
    regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
    regs.cpsr().set_ccf(ccf::z, not res);
    regs.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));
}

template<immediate_operand I, s_bit S, shifts Shift, arithmetic_operation Operation>
auto instruction_executor::arm_arithmetic(arm7tdmi& cpu, u32 instruction) -> void {
    auto const rd = (instruction >> 12) & 0xf;
    auto const rn = (instruction >> 16) & 0xf;
    auto& regs = cpu.m_registers;
    auto& destination = regs[rd];
    auto const operand2 = cpu::i_have_no_clue_how_to_name_this<I, s_bit::off, Shift>(cpu, instruction);
    [[maybe_unused]]auto const carryout = Operation(regs[rn], operand2, &destination, regs.cpsr().check_ccf(ccf::c)); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, carryout);
        regs.cpsr().set_ccf(ccf::z, not destination);
        regs.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));
        regs.cpsr().set_ccf(ccf::v, cpu::check_overflow(regs[rn], operand2, destination));
    }
}
template<immediate_operand I, shifts Shift, arithmetic_operation Operation>
auto instruction_executor::arm_arithmetic(arm7tdmi& cpu, u32 instruction) -> void {
    auto const rn = (instruction >> 16) & 0xf;
    auto& regs = cpu.m_registers;
    u32 destination;
    auto const operand2 = cpu::i_have_no_clue_how_to_name_this<I, s_bit::off, Shift>(cpu, instruction);
    auto const carryout = Operation(regs[rn], operand2, &destination, regs.cpsr().check_ccf(ccf::c)); 
    regs.cpsr().set_ccf(ccf::c, carryout);
    regs.cpsr().set_ccf(ccf::z, not destination);
    regs.cpsr().set_ccf(ccf::n, destination & (1_u32 << 31));
    regs.cpsr().set_ccf(ccf::v, cpu::check_overflow(regs[rn], operand2, destination));
}

template<immediate_operand I, s_bit S, shifts Shift, single_operand_operation Operation>
auto instruction_executor::arm_single_operand(arm7tdmi& cpu, u32 instruction) -> void {
    auto const rd = (instruction >> 12) & 0xf;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, S, Shift>(cpu, instruction);
    auto const operand = cpu::get_operand2<S>(shifted_result);
    [[maybe_unused]]auto const res = regs[rd] = Operation(operand); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
        regs.cpsr().set_ccf(ccf::z, not res);
        regs.cpsr().set_ccf(ccf::n, res & (1_u32 << 31));
    }
}

}


#endif
