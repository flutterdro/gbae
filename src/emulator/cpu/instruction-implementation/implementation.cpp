#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpu/instruction-impl/detail/instruction-executor.hpp"
#include "emulator/cpu/lla.hpp"
#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpudefines.hpp"
#include <array>
#include "emulator/cpu/instruction-impl/implementation.hpp"
//You know shits about to get down when you see this
//NOLINTBEGIN
namespace fgba::cpu {

namespace  {

[[nodiscard]]constexpr auto bic_impl(u32 operand1, u32 operand2) noexcept { return operand1 & (~operand2); }
[[nodiscard]]constexpr auto and_impl(u32 operand1, u32 operand2) noexcept { return operand1 & operand2; }
[[nodiscard]]constexpr auto eor_impl(u32 operand1, u32 operand2) noexcept { return operand1 ^ operand2; }
[[nodiscard]]constexpr auto orr_impl(u32 operand1, u32 operand2) noexcept { return operand1 | operand2; }
[[nodiscard]]constexpr auto mvn_impl(u32 operand1)               noexcept { return ~operand1; }
[[nodiscard]]constexpr auto mov_impl(u32 operand1)               noexcept { return operand1; }


using impl_ptr = auto (*)(arm7tdmi&, u32) -> void;
using impl_array = std::array<impl_ptr, arm_instruction::count()>;

enum class ignore_dest {
    off = 0,
    on  = 1,
};

template<arithmetic_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, u32 instr) -> void { 
    if constexpr (Id == ignore_dest::off) {
        instruction_executor::arm_arithmetic<O, S, Shift, Op>(cpu, instr);
    } else {
        instruction_executor::arm_arithmetic<O, Shift, Op>(cpu, instr);
    }
}
template<logical_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, u32 instr) -> void { 
    if constexpr (Id == ignore_dest::off) {
        instruction_executor::arm_logical<O, S, Shift, Op>(cpu, instr);
    } else {
        instruction_executor::arm_logical<O, Shift, Op>(cpu, instr);
    }
}
template<single_operand_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, u32 instr) -> void { 
    instruction_executor::arm_single_operand<O, S, Shift, Op>(cpu, instr);
}

template<auto Operation, arm_instruction::set Instr, ignore_dest Id, shifts Shift>
consteval auto bind_operation_with_shift(impl_array& arr) -> void {
    if constexpr (Id == ignore_dest::off) {
        arr[arm_instruction::construct<Instr>(
            immediate_operand::off,
            Shift,
            s_bit::off
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::off, Id, s_bit::off, Shift>;
        arr[arm_instruction::construct<Instr>(
            immediate_operand::on,
            Shift,
            s_bit::off
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::on, Id, s_bit::off, Shift>;
        arr[arm_instruction::construct<Instr>(
            immediate_operand::off,
            Shift,
            s_bit::on
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::off, Id, s_bit::on, Shift>;
        arr[arm_instruction::construct<Instr>(
            immediate_operand::on,
            Shift,
            s_bit::on
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::on, Id, s_bit::on, Shift>;
    } else {
        arr[arm_instruction::construct<Instr>(
            immediate_operand::off,
            Shift
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::off, Id, s_bit::off, Shift>;
        arr[arm_instruction::construct<Instr>(
            immediate_operand::on,
            Shift
        ).as_index()] = &cpu::arm_overload<Operation, immediate_operand::on, Id, s_bit::off, Shift>;
    }
}
template<auto Operation, arm_instruction::set Instr, ignore_dest Id> 
consteval auto bind_operation(impl_array& arr) -> void {
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::null>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::asr32>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::lsr32>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::rrx>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::lsl>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::lsr>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::asr>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::ror>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::rslsl>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::rslsr>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::rsasr>(arr);   
    cpu::bind_operation_with_shift<Operation, Instr, Id, shifts::rsror>(arr);   
}
consteval auto init_arm_impl_ptrs() {
    impl_array ret{};

    ret[arm_instruction::construct<arm_instruction::set::b>().as_index()] = instruction_executor::arm_b;
    ret[arm_instruction::construct<arm_instruction::set::bl>().as_index()] = instruction_executor::arm_bl;
    ret[arm_instruction::construct<arm_instruction::set::bx>().as_index()] = instruction_executor::arm_bx;
    cpu::bind_operation<eor_impl, arm_instruction::set::eor, ignore_dest::off>(ret);
    cpu::bind_operation<and_impl, arm_instruction::set::and_, ignore_dest::off>(ret);
    cpu::bind_operation<orr_impl, arm_instruction::set::orr, ignore_dest::off>(ret);
    cpu::bind_operation<bic_impl, arm_instruction::set::bic, ignore_dest::off>(ret);
    cpu::bind_operation<add_impl, arm_instruction::set::add, ignore_dest::off>(ret);
    cpu::bind_operation<adc_impl, arm_instruction::set::adc, ignore_dest::off>(ret);
    cpu::bind_operation<sub_impl, arm_instruction::set::sub, ignore_dest::off>(ret);
    cpu::bind_operation<sbc_impl, arm_instruction::set::sbc, ignore_dest::off>(ret);
    cpu::bind_operation<rsb_impl, arm_instruction::set::rsb, ignore_dest::off>(ret);
    cpu::bind_operation<rsc_impl, arm_instruction::set::rsc, ignore_dest::off>(ret);
    cpu::bind_operation<and_impl, arm_instruction::set::tst, ignore_dest::on>(ret);
    cpu::bind_operation<eor_impl, arm_instruction::set::teq, ignore_dest::on>(ret);
    cpu::bind_operation<sub_impl, arm_instruction::set::cmp, ignore_dest::on>(ret);
    cpu::bind_operation<add_impl, arm_instruction::set::cmn, ignore_dest::on>(ret);
    return ret;
}
inline constexpr impl_array arm_impl_ptrs = init_arm_impl_ptrs();

}



}
//NOLINTEND
auto fgba::cpu::execute_arm(arm7tdmi& cpu, arm_instruction instruction, u32 opcode) -> void {
    std::invoke(arm_impl_ptrs[instruction.as_index()], cpu, opcode);
}
