#ifndef INSTRUCTION_EXECUTION_HPP_KCNFKJCN
#define INSTRUCTION_EXECUTION_HPP_KCNFKJCN

#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpudefines.hpp"
#include "emulator/cpu/arm7tdmi.hpp"
#include "fgba-defines.hpp"
#include "emulator/cpu/shifter.hpp"

namespace fgba::cpu {


using single_operand_operation = auto(*)(u32) -> u32;
using logical_operation        = auto(*)(u32, u32) -> u32;
using arithmetic_operation     = auto(*)(u32, u32, u32*, u32) -> bool;
struct instruction_executor {
    static auto arm_b(arm7tdmi&, u32)
        -> void;
    static auto arm_bl(arm7tdmi&, u32)
        -> void;
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
    template<s_bit S, accumulate A>
    static auto arm_multiply(arm7tdmi&, u32 instruction)
        -> void;
    template<s_bit S, accumulate A, mll_signedndesd MS>
    static auto arm_multiply_long(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand I, which_psr P, mask M>
    static auto arm_move_to_psr(arm7tdmi&, u32 instruction)
        -> void;
    template<which_psr P>
    static auto arm_move_from_psr(arm7tdmi&, u32 instruction)
        -> void;
    template<indexing Ind>
    static auto arm_data_store(arm7tdmi&, u32 instruction)
        -> void;
    template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
    static auto arm_block_data_store(arm7tdmi&, u32 instruction)
        -> void;
};
} //namespace fgba::cpu

namespace fgba::cpu {

inline auto instruction_executor::arm_b(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto const offset = [&] -> u32 {
        auto const packed_offset = instruction & 0xffffff_u32;
        return static_cast<u32>(signextend<24>(packed_offset << 2_u32));
    }();
    
    cpu.m_registers.pc() += offset;
    cpu.flush_pipeline();
    cpu.prefetch();
    cpu.m_registers.pc() += 4_u32;
    cpu.prefetch();
}

inline auto instruction_executor::arm_bl(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto const offset = [&] -> u32 {
        auto const packed_offset = instruction & 0xffffff_u32;
        return signextend<24>(packed_offset << 2_u32);
    }();
    
    cpu.m_registers.lr() = cpu.m_registers.pc() - 4_u32;
    
    cpu.m_registers.pc() += offset;
    cpu.flush_pipeline();
    cpu.prefetch();
    cpu.m_registers.pc() += 4_u32;
    cpu.prefetch();
}

inline auto instruction_executor::arm_bx(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto const rn = instruction & 0xf_u32;
    auto& regs = cpu.m_registers;
    auto should_switch_to_thumb = (regs[rn] & 0b1_u32) == 1_u32;
    regs.cpsr().use_thumb(should_switch_to_thumb); 
    cpu.flush_pipeline();
    regs.pc() = regs[rn];
    cpu.prefetch();
    cpu.increment_program_counter();
    cpu.prefetch();
}



[[nodiscard]]constexpr auto check_overflow(u32 operand1, u32 operand2, u32 result) {
    return ((operand1 >> 31u) == (operand2 >> 31u)) && ((operand1 >> 31u) != (result >> 31u));
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

template<s_bit S, accumulate A>
auto instruction_executor::arm_multiply(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto const rd = instruction >> 16 & 0xf;
    [[maybe_unused]]
    auto const rn = instruction >> 12 & 0xf;
    auto const rs = instruction >>  8 & 0xf;
    auto const rm = instruction >>  0 & 0xf;

    auto& regs = cpu.m_registers;

    auto accumulated = u32{0};
    if constexpr (A == accumulate::on) accumulated = regs[rn];

    auto const mul_res = regs[rd] = regs[rm] * regs[rs] + accumulated;

    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::n, mul_res & 1_u32 << 31);
        regs.cpsr().set_ccf(ccf::z, not mul_res);
        regs.cpsr().set_ccf(ccf::c, 0);
    }
}

template<s_bit S, accumulate A, mll_signedndesd MS>
auto instruction_executor::arm_multiply_long(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto const rdhi = instruction >> 16 & 0xf;
    auto const rdlo = instruction >> 12 & 0xf;
    auto const rs   = instruction >>  8 & 0xf;
    auto const rm   = instruction >>  0 & 0xf;

    auto& regs = cpu.m_registers;
    // I am feeling a bit quirky today
    // Also I am just using unsigned instead of seperating by signdendess
    // I really can't wrap my head around why they are different for now
    auto accumulated = u64{0};
    if constexpr (A == accumulate::on) accumulated = u64{regs[rdhi]} << 32 | u64{regs[rdlo]};
    auto const operand1 = u64{regs[rs]};
    auto const operand2 = u64{regs[rm]};

    auto const mll_res  = operand1 * operand2 + accumulated;
    regs[rdlo] = mll_res       & 0xffffffff;
    regs[rdhi] = mll_res >> 31 & 0xffffffff;

    if constexpr (S == s_bit::on) {
        regs.cpsr().set_ccf(ccf::n, mll_res & 1_u64 << 63);
        regs.cpsr().set_ccf(ccf::z, not mll_res);
        // for now I will just set c and v to zero
        regs.cpsr().set_ccf(ccf::c, 0);
        regs.cpsr().set_ccf(ccf::v, 0);
    }
}


template<which_psr P>
auto instruction_executor::arm_move_from_psr(arm7tdmi& cpu, u32 instruction)
    -> void {
    auto& regs = cpu.m_registers;
    auto const rd = instruction >> 12 & 0xf;

    if constexpr (P == which_psr::cpsr) {
        regs[rd] = regs.cpsr().val;
    } else {
        regs[rd] = regs.spsr().val;
    }
}
template<immediate_operand I, which_psr P, mask M>
auto instruction_executor::arm_move_to_psr(arm7tdmi& cpu, u32 instruction)
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

template<immediate_operand Im, shifts Shift, direction Dir, indexing Ind, write_back Wb, data_size Data>
static auto arm_data_store(arm7tdmi& cpu, u32 instruction) 
    -> void {
    auto& regs = cpu.m_registers;
    auto const offset = [&] -> u32 {
        auto const abs_offset = 
            (Im == immediate_operand::on ? 
                (Data == data_size::hword ?
                    (instruction >> 4_u32 & 0xf_u32) | (instruction & 0xf_u32) :
                    (instruction & 0xfff_u32)
                ) :
                (cpu::i_have_no_clue_how_to_name_this<immediate_operand::off, s_bit::off, Shift>(cpu, instruction))
            );
        auto const adjusted_sign_offset = 
            Dir == direction::up ?
                abs_offset :
               -abs_offset;
        return adjusted_sign_offset;
    }();

    auto const data_to_load_on_bus = [&] -> u32 {
        auto const data_to_transfer = regs[instruction >> 12_u32 & 0xf_u32];
        if constexpr (Data == data_size::byte) {
            auto const byte = data_to_transfer & 0xff_u32;
            return byte | byte << 8_u32 | byte << 16_u32 | byte << 24_u32;
        } else if constexpr (Data == data_size::hword) {
            auto const hword = data_to_transfer & 0xffff_u32;
            return hword | hword << 16_u32;
        }
        return data_to_transfer;
    }();
    cpu.m_bus.load_on(data_to_load_on_bus);

    auto const r_b = instruction >> 16_u32 & 0xf_u32;
    if constexpr (Ind == indexing::post) {
        cpu.m_bus.access_write(address{regs[r_b]}, Data);
    } else {
        cpu.m_bus.access_write(address{regs[r_b] + offset}, Data);
    }

    if constexpr (Wb == write_back::on or Ind == indexing::post) {
        regs[r_b] += offset;
    }
}


}


#endif
