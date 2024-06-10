#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpu/instruction-impl/detail/instruction-executor.hpp"
#include "emulator/cpu/lla.hpp"
#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpudefines.hpp"
#include <array>
#include "emulator/cpu/instruction-impl/implementation.hpp"
//You know shits about to get down when you see this
//NOLINTBEGIN
namespace fgba::cpu::arm {

namespace  {

[[nodiscard]]constexpr auto bic_impl(word operand1, word operand2) noexcept { return operand1 & (~operand2); }
[[nodiscard]]constexpr auto and_impl(word operand1, word operand2) noexcept { return operand1 & operand2; }
[[nodiscard]]constexpr auto eor_impl(word operand1, word operand2) noexcept { return operand1 ^ operand2; }
[[nodiscard]]constexpr auto orr_impl(word operand1, word operand2) noexcept { return operand1 | operand2; }
[[nodiscard]]constexpr auto mvn_impl(word operand1)                noexcept { return ~operand1; }
[[nodiscard]]constexpr auto mov_impl(word operand1)                noexcept { return operand1; }


using impl_ptr = auto (*)(arm7tdmi&, instruction) -> void;
using impl_array = std::array<impl_ptr, instruction_spec::count()>;

enum class ignore_dest {
    off = 0,
    on  = 1,
};

template<arithmetic_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, instruction instr) -> void { 
    if constexpr (Id == ignore_dest::off) {
        instruction_executor::arithmetic<O, S, Shift, Op>(cpu, instr);
    } else {
        instruction_executor::arithmetic<O, Shift, Op>(cpu, instr);
    }
}
template<logical_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, instruction instr) -> void { 
    if constexpr (Id == ignore_dest::off) {
        instruction_executor::logical<O, S, Shift, Op>(cpu, instr);
    } else {
        instruction_executor::logical<O, Shift, Op>(cpu, instr);
    }
}
template<single_operand_operation Op, immediate_operand O, ignore_dest Id, s_bit S, shifts Shift>
auto arm_overload(arm7tdmi& cpu, instruction instr) -> void { 
    instruction_executor::single_operand<O, S, Shift, Op>(cpu, instr);
}

template<auto Operation, instruction_spec::set Instr, ignore_dest Id, shifts Shift>
consteval auto bind_operation_with_shift(impl_array& arr) -> void {
    if constexpr (Id == ignore_dest::off) {
        arr[instruction_spec::construct<Instr>(
            immediate_operand::off,
            Shift,
            s_bit::off
        ).as_index()] = &arm_overload<Operation, immediate_operand::off, Id, s_bit::off, Shift>;
        arr[instruction_spec::construct<Instr>(
            immediate_operand::on,
            Shift,
            s_bit::off
        ).as_index()] = &arm_overload<Operation, immediate_operand::on, Id, s_bit::off, Shift>;
        arr[instruction_spec::construct<Instr>(
            immediate_operand::off,
            Shift,
            s_bit::on
        ).as_index()] = &arm_overload<Operation, immediate_operand::off, Id, s_bit::on, Shift>;
        arr[instruction_spec::construct<Instr>(
            immediate_operand::on,
            Shift,
            s_bit::on
        ).as_index()] = &arm_overload<Operation, immediate_operand::on, Id, s_bit::on, Shift>;
    } else {
        arr[instruction_spec::construct<Instr>(
            immediate_operand::off,
            Shift
        ).as_index()] = &arm_overload<Operation, immediate_operand::off, Id, s_bit::off, Shift>;
        arr[instruction_spec::construct<Instr>(
            immediate_operand::on,
            Shift
        ).as_index()] = &arm_overload<Operation, immediate_operand::on, Id, s_bit::off, Shift>;
    }
}
template<auto Operation, instruction_spec::set Instr, ignore_dest Id> 
consteval auto bind_operation(impl_array& arr) -> void {
    bind_operation_with_shift<Operation, Instr, Id, shifts::null>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::asr32>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::lsr32>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::rrx>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::lsl>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::lsr>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::asr>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::ror>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::rslsl>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::rslsr>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::rsasr>(arr);   
    bind_operation_with_shift<Operation, Instr, Id, shifts::rsror>(arr);   
}
consteval auto init_arm_impl_ptrs() {
    impl_array ret{};

    ret[instruction_spec::construct<instruction_spec::set::b>().as_index()] = instruction_executor::b;
    ret[instruction_spec::construct<instruction_spec::set::bl>().as_index()] = instruction_executor::bl;
    ret[instruction_spec::construct<instruction_spec::set::bx>().as_index()] = instruction_executor::bx;
    bind_operation<eor_impl, instruction_spec::set::eor, ignore_dest::off>(ret);
    bind_operation<and_impl, instruction_spec::set::and_, ignore_dest::off>(ret);
    bind_operation<orr_impl, instruction_spec::set::orr, ignore_dest::off>(ret);
    bind_operation<bic_impl, instruction_spec::set::bic, ignore_dest::off>(ret);
    bind_operation<add_impl, instruction_spec::set::add, ignore_dest::off>(ret);
    bind_operation<adc_impl, instruction_spec::set::adc, ignore_dest::off>(ret);
    bind_operation<sub_impl, instruction_spec::set::sub, ignore_dest::off>(ret);
    bind_operation<sbc_impl, instruction_spec::set::sbc, ignore_dest::off>(ret);
    bind_operation<rsb_impl, instruction_spec::set::rsb, ignore_dest::off>(ret);
    bind_operation<rsc_impl, instruction_spec::set::rsc, ignore_dest::off>(ret);
    bind_operation<and_impl, instruction_spec::set::tst, ignore_dest::on>(ret);
    bind_operation<eor_impl, instruction_spec::set::teq, ignore_dest::on>(ret);
    bind_operation<sub_impl, instruction_spec::set::cmp, ignore_dest::on>(ret);
    bind_operation<add_impl, instruction_spec::set::cmn, ignore_dest::on>(ret);
    return ret;
}
inline constexpr impl_array arm_impl_ptrs = init_arm_impl_ptrs();

}



}
//NOLINTEND
namespace fgba::cpu::arm {
auto arm::execute(arm7tdmi& cpu, instruction_spec spec, instruction instruction) -> void {
    std::invoke(arm_impl_ptrs[spec.as_index()], cpu, instruction);
}
}
