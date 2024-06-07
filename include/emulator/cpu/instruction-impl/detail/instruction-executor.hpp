#ifndef INSTRUCTION_EXECUTION_HPP_KCNFKJCN
#define INSTRUCTION_EXECUTION_HPP_KCNFKJCN

#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpudefines.hpp"
#include "emulator/cpu/arm7tdmi.hpp"
#include "fgba-defines.hpp"
#include "emulator/cpu/shifter.hpp"
#include <bit>
#include <utility>

namespace fgba::cpu {


using single_operand_operation = auto(*)(word) -> word;
using logical_operation        = auto(*)(word, word) -> word;
using arithmetic_operation     = auto(*)(word, word, word*, u32) -> bool;
struct instruction_executor {
    static auto arm_b(arm7tdmi&, word)
        -> void;
    static auto arm_bl(arm7tdmi&, word)
        -> void;
    static auto arm_bx(arm7tdmi& cpu, word instruction) 
        -> void; 
    template<immediate_operand I, s_bit S, shifts Shift, arithmetic_operation Operation>
    static auto arm_arithmetic(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand I, shifts Shift, arithmetic_operation Operation>
    static auto arm_arithmetic(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand I, s_bit S, shifts Shift, logical_operation Operation>
    static auto arm_logical(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand I, shifts Shift, logical_operation Operation>
    static auto arm_logical(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand I, s_bit S, shifts Shift, single_operand_operation Operation>
    static auto arm_single_operand(arm7tdmi&, word instruction) 
        -> void;
    template<s_bit S, accumulate A>
    static auto arm_multiply(arm7tdmi&, word instruction)
        -> void;
    template<s_bit S, accumulate A, mll_signedndesd MS>
    static auto arm_multiply_long(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand I, which_psr P, mask M>
    static auto arm_move_to_psr(arm7tdmi&, word instruction)
        -> void;
    template<which_psr P>
    static auto arm_move_from_psr(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
    static auto arm_data_store(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
    static auto arm_data_load(arm7tdmi&, word instruction)
        -> void;
    template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
    static auto arm_block_data_store(arm7tdmi&, word instruction)
        -> void;
};
} //namespace fgba::cpu

namespace fgba::cpu {

inline auto instruction_executor::arm_b(arm7tdmi& cpu, word const instruction) 
    -> void {
    auto const offset = instruction[23, 0].sign_extend<23>().lsl(2);
    
    cpu.m_registers.pc() += offset;
    cpu.flush_pipeline();
    cpu.prefetch();
    cpu.m_registers.pc() += 4_word;
    cpu.prefetch();
}

inline auto instruction_executor::arm_bl(arm7tdmi& cpu, word const instruction)
    -> void {
    auto const offset = instruction[23, 0].sign_extend<23>().lsl(2);    
    cpu.m_registers.lr() = cpu.m_registers.pc() - 4_word;
    
    cpu.m_registers.pc() += offset;
    cpu.flush_pipeline();
    cpu.prefetch();
    cpu.m_registers.pc() += 4_word;
    cpu.prefetch();
}

inline auto instruction_executor::arm_bx(arm7tdmi& cpu, word const instruction)
    -> void {
    auto const rn = instruction[3, 0];
    auto& regs = cpu.m_registers;
    auto should_switch_to_thumb = regs[rn.value][0] == 1_bit;
    regs.cpsr().use_thumb(should_switch_to_thumb); 
    cpu.flush_pipeline();
    regs.pc() = regs[rn.value];
    cpu.prefetch();
    cpu.increment_program_counter();
    cpu.prefetch();
}



[[nodiscard]]constexpr auto check_overflow(word const operand1, word const operand2, word const result) {
    return (operand1[31] == operand2[31]) and (operand1[31] != result[31]);
}

template<s_bit S>
constexpr auto get_operand2(auto const shifted) -> word {
    if constexpr (S == s_bit::on) return shifted.shifted_data;
    else return shifted;
}

template<immediate_operand I, s_bit S, shifts Shift>
FGBA_FORCE_INLINE
constexpr auto i_have_no_clue_how_to_name_this(arm7tdmi const& cpu, word const instruction) {
    auto const& regs = cpu.m_registers;
    if constexpr (I == immediate_operand::on) {
        auto const shift_operand = instruction[7, 0];
        auto const shift_amount  = 2 * instruction[11, 8].value;
        auto const result        = shift_operand.ror(shift_amount);
        if constexpr (S == s_bit::on) {
            auto const carryout = shift_amount == 0 ? 
                regs.cpsr().check_ccf(ccf::c) :
                result[31];
            return shifter::shift_res{result, carryout};
        } else {
            return result;
        }
    } else {
        auto const rm = instruction[3, 0].value;
        return shifter::barrel<Shift, S>(instruction, regs[rm], regs);
    }
}

template<immediate_operand I, s_bit S, shifts Shift, logical_operation Operation>
auto instruction_executor::arm_logical(arm7tdmi& cpu, word const instruction) -> void {
    auto const rd = instruction[15, 12].value;
    auto const rn = instruction[19, 16].value;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, S, Shift>(cpu, instruction);
    auto const operand2 = cpu::get_operand2<S>(shifted_result);
    [[maybe_unused]]auto const res = regs[rd] = Operation(regs[rn], operand2); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
        regs.cpsr().set_ccf(ccf::z, res == 0_word);
        regs.cpsr().set_ccf(ccf::n, res[31]);
    }
    
}

template<immediate_operand I, shifts Shift, logical_operation Operation>
auto instruction_executor::arm_logical(arm7tdmi& cpu, word const instruction) -> void {
    auto const rn = instruction[19, 16].value;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, s_bit::on, Shift>(cpu, instruction);
    auto const operand2 = cpu::get_operand2<s_bit::on>(shifted_result);
    auto const res = Operation(regs[rn], operand2); 
    regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
    regs.cpsr().set_ccf(ccf::z, res == 0_word);
    regs.cpsr().set_ccf(ccf::n, res[31]);
}

template<immediate_operand I, s_bit S, shifts Shift, arithmetic_operation Operation>
auto instruction_executor::arm_arithmetic(arm7tdmi& cpu, word const instruction) -> void {
    auto const rd = instruction[15, 12].value;
    auto const rn = instruction[19, 16].value;
    auto& regs = cpu.m_registers;
    auto& destination = regs[rd];
    auto const operand2 = cpu::i_have_no_clue_how_to_name_this<I, s_bit::off, Shift>(cpu, instruction);
    [[maybe_unused]]auto const carryout = Operation(regs[rn], operand2, &destination, regs.cpsr().check_ccf(ccf::c)); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, carryout);
        regs.cpsr().set_ccf(ccf::z, destination == 0_word);
        regs.cpsr().set_ccf(ccf::n, destination[31] == 1_bit);
        regs.cpsr().set_ccf(ccf::v, cpu::check_overflow(regs[rn], operand2, destination));
    }
}
template<immediate_operand I, shifts Shift, arithmetic_operation Operation>
auto instruction_executor::arm_arithmetic(arm7tdmi& cpu, word const instruction) -> void {
    auto const rn = instruction[19, 16].value;
    auto& regs = cpu.m_registers;
    word destination;
    auto const operand2 = cpu::i_have_no_clue_how_to_name_this<I, s_bit::off, Shift>(cpu, instruction);
    auto const carryout = Operation(regs[rn], operand2, &destination, regs.cpsr().check_ccf(ccf::c)); 
    regs.cpsr().set_ccf(ccf::c, carryout);
    regs.cpsr().set_ccf(ccf::z, destination == 0_word);
    regs.cpsr().set_ccf(ccf::n, destination[31]);
    regs.cpsr().set_ccf(ccf::v, cpu::check_overflow(regs[rn], operand2, destination));
}

template<immediate_operand I, s_bit S, shifts Shift, single_operand_operation Operation>
auto instruction_executor::arm_single_operand(arm7tdmi& cpu, word const instruction) -> void {
    auto const rd = instruction[15, 12].value;
    auto& regs = cpu.m_registers;
    auto const shifted_result = cpu::i_have_no_clue_how_to_name_this<I, S, Shift>(cpu, instruction);
    auto const operand = cpu::get_operand2<S>(shifted_result);
    [[maybe_unused]]auto const res = regs[rd] = Operation(operand); 
    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::c, shifted_result.carryout);
        regs.cpsr().set_ccf(ccf::z, res == 0_word);
        regs.cpsr().set_ccf(ccf::n, res[31]);
    }
}

template<s_bit S, accumulate A>
auto instruction_executor::arm_multiply(arm7tdmi& cpu, word const instruction)
    -> void {
    auto const rd = instruction[19, 16].value;
    [[maybe_unused]]
    auto const rn = instruction[15, 12].value;
    auto const rs = instruction[11,  8].value;
    auto const rm = instruction[ 3,  0].value;

    auto& regs = cpu.m_registers;

    auto const accumulated = A == accumulate::on ? regs[rn] : 0_word;

    auto const mul_res = regs[rd] = regs[rm] * regs[rs] + accumulated;

    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::n, mul_res[31] == 1_bit);
        regs.cpsr().set_ccf(ccf::z, mul_res == 0_word);
        regs.cpsr().set_ccf(ccf::c, 0);
    }
}

template<s_bit S, accumulate A, mll_signedndesd MS>
auto instruction_executor::arm_multiply_long(arm7tdmi& cpu, word const instruction)
    -> void {
    auto const rdhi = instruction[19, 16].value;
    auto const rdlo = instruction[15, 12].value;
    auto const rs   = instruction[11,  8].value;
    auto const rm   = instruction[ 3,  0].value;

    auto& regs = cpu.m_registers;
    // I am feeling a bit quirky today 
    // Also I am just using unsigned instead of seperating by signdendess
    // I really can't wrap my head around why they are different for now
    auto const accumulated = A == accumulate::on ? 
        regs[rdhi].as<dword>().lsl(32) | regs[rdlo].as<dword>() :
        0_dword;
    auto const operand1 = MS == mll_signedndesd::signed_ ?
        regs[rs].as<dword>().sign_extend<31>() :
        regs[rs].as<dword>();
    auto const operand2 = MS == mll_signedndesd::signed_ ?
        regs[rm].as<dword>().sign_extend<31>() :
        regs[rm].as<dword>();

    auto const mll_res  = operand1 * operand2 + accumulated;
    regs[rdlo] = mll_res.as<word>();
    regs[rdhi] = mll_res.lsl(32).as<word>();

    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::n, mll_res[63] == 1_bit);
        regs.cpsr().set_ccf(ccf::z, mll_res != 0_dword);
        // for now I will just set c and v to zero
        regs.cpsr().set_ccf(ccf::c, 0);
        regs.cpsr().set_ccf(ccf::v, 0);
    }
}


template<which_psr P>
auto instruction_executor::arm_move_from_psr(arm7tdmi& cpu, word const instruction)
    -> void {
    auto& regs = cpu.m_registers;
    auto const rd = instruction[15, 12].value;

    if constexpr (P == which_psr::cpsr) {
        regs[rd] = {regs.cpsr().val};
    } else {
        regs[rd] = {regs.spsr().val};
    }
}
template<immediate_operand I, which_psr P, mask M>
auto instruction_executor::arm_move_to_psr(arm7tdmi& cpu, word const instruction)
    -> void {
    auto& regs = cpu.m_registers;
    auto const operand = cpu::i_have_no_clue_how_to_name_this<I, s_bit::off, shifts::null>(cpu, instruction);

    auto mask = u32{0};
    if constexpr (M == mask::on) operand &= ~(mask = 0xf_u32 << 27);

    if constexpr (P == which_psr::cpsr) {
        regs.cpsr().val = (regs.cpsr().val & ~mask) | operand;
    } else {
        regs.spsr().val = (regs.spsr().val & ~mask) | operand;
    }

}


template<immediate_operand Im, shifts Shift, direction Dir, data_size Data>
auto extract_offset(arm7tdmi& cpu, word const instruction) 
    -> u32 {
    auto const absolute_offset = (
        Im == immediate_operand::on ? (
            Data == data_size::hword or Data == data_size::byte ? (
                instruction[11, 8].lsl(4) | instruction[3, 0] 
            ) : (
                instruction[11, 0]
            )
        ) : (
            (cpu::i_have_no_clue_how_to_name_this<immediate_operand::off, s_bit::off, Shift>(cpu, instruction))
        )
    );
    auto const adjusted_sign_offset = (
        Dir == direction::up ? (
            absolute_offset
        ) : (
           -absolute_offset
        )
    );
    return adjusted_sign_offset;
}

template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
auto instruction_executor::arm_data_store(arm7tdmi& cpu, word const instruction) 
    -> void {
    auto& regs = cpu.m_registers;
    auto const offset = cpu::extract_offset<Im, Shift, Dir, Data>(cpu, instruction);
    auto const data_to_load_on_bus = [&] -> word {
        word result;
        auto const data_to_transfer = regs[instruction[15, 12].value];
        if constexpr (Data == data_size::byte) {
            return result[31, 24] = result[23, 16] = result[15, 8] = result[7, 0] = data_to_transfer[7, 0];
        } else if constexpr (Data == data_size::hword) {
            return result[31, 16] = result[15, 0] = data_to_transfer[15, 0];
        }
        return data_to_transfer;
    }();
    cpu.m_bus.load_on(data_to_load_on_bus);

    auto const r_b = instruction[19, 16];
    if constexpr (Ind == indexing::post) {
        cpu.m_bus.access_write(address{regs[r_b.value].value}, Data);
    } else {
        cpu.m_bus.access_write(address{(regs[r_b.value] + offset).value}, Data);
    }

    if constexpr (Wb == write_back::on or Ind == indexing::post) {
        regs[r_b.value] += offset;
    }
}

consteval auto mask_for(data_size data_size) 
    -> u32 {
    switch (data_size) {
        case data_size::byte:  return 0xff_u32;
        case data_size::hword: return 0xffff_u32;
        case data_size::word:  return 0xffffffff_u32;
        default: std::unreachable();
    }
}
template<data_size Data, mll_signedndesd Sign>
auto process_data_from_bus(u32 data, address address) 
    -> u32 {
    auto const alignment      = address.value & 0b11_u32;
    auto const realigned_data = std::rotr(data, alignment * 8_u32);
    auto const masked_data    = realigned_data & mask_for(Data);

    if (Sign == mll_signedndesd::signed_) {

    }
}

template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
auto instruction_executor::arm_data_load(arm7tdmi& cpu, word const instruction)
    -> void {
    auto& regs = cpu.m_registers;
    auto const offset = cpu::extract_offset<Im, Shift, Dir, Data>(cpu, instruction);
    auto const r_base = instruction[19, 16];

    auto const source_address = [&] -> address {
        return Ind == indexing::post ? regs[r_base.value] : regs[r_base.value] + offset;
    }();

    cpu.m_bus.access_read(source_address, Data);
    auto data = cpu.m_bus.load_from();


}



}


#endif
