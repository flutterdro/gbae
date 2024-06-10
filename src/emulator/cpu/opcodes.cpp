#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpudefines.hpp"
#include <array>
#include <tuple>
#include <utility>

namespace fgba {

namespace cpu {

template<arm_instruction::set Instr>
consteval auto test_extraction_and_uniqueness(auto... switches) 
    -> bool {
    return std::tuple{switches...} == arm_instruction::construct<Instr>(switches...).template switches<Instr>();
}
static_assert(test_extraction_and_uniqueness<arm_instruction::set::b>());
static_assert(test_extraction_and_uniqueness<arm_instruction::set::bl>());
static_assert(test_extraction_and_uniqueness<arm_instruction::set::bx>());
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::off,
    shifts::null,
    s_bit::off
));
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::on,
    shifts::null,
    s_bit::off
));
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::off,
    shifts::null,
    s_bit::on
));
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::off,
    shifts::null,
    s_bit::off
));
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::off,
    shifts::rsasr,
    s_bit::off
));
static_assert(test_extraction_and_uniqueness<arm_instruction::set::and_>(
    immediate_operand::off,
    shifts::rsasr,
    s_bit::on
));
}

namespace {

using enum cpu::arm_instruction::set;
struct arm_instruction_info {
    word mask;
    word opcode;
};
using arm_lut = std::array<arm_instruction_info, cpu::arm_instruction::count()>;


//dummy namespace just to qualify functions
namespace qual {

consteval auto generate_arm_opcode_lut()
    -> arm_lut;


consteval auto asign_arm_branch_opcodes(arm_lut& lut)
    -> void;


consteval auto asign_arm_data_processing(arm_lut& lut)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_and_like(arm_lut&, arm_instruction_info, word id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_tst_like(arm_lut&, arm_instruction_info, word id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_s_bit_variation(arm_lut&, arm_instruction_info, cpu::s_bit)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_special_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, word id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_register_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, word id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_normal_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, word id)
    -> void;


consteval auto asign_arm_psr_transfer(arm_lut&)
    -> void;
consteval auto arm_create_mrs(arm_lut&)
    -> void;
consteval auto arm_create_msr(arm_lut&)
    -> void;
consteval auto arm_create_msr_full(arm_lut&)
    -> void;
consteval auto arm_create_msr_flag_bits(arm_lut&)
    -> void;
} // namespace qual

consteval auto qual::asign_arm_branch_opcodes(arm_lut& lut)
    -> void {
    lut[cpu::arm_instruction::construct<b>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'1010'0000'0000'0000'0000'0000'0000_word,
    };
    lut[cpu::arm_instruction::construct<bl>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'1011'0000'0000'0000'0000'0000'0000_word,
    };
    lut[cpu::arm_instruction::construct<bx>().as_index()] = {
        .mask   = 0b0000'1111'1111'1111'1111'1111'1111'0000_word,
        .opcode = 0b0000'0001'0010'1111'1111'1111'0001'0000_word,
    };
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_special_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[11, 4]    = 0b1111'1111_word;
    base.opcode[6, 5]   = id;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_register_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[7, 4]   = 0b1111_word;
    base.opcode[6, 5] = id;
    base.opcode[4]    = 1_bit;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_normal_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[6, 4]   = 0b111_word;
    base.opcode[6, 5] = id;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_s_bit_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s)
    -> void {
    base.opcode[20] = s == cpu::s_bit::on ? 1_bit : 0_bit;
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::null,  0b00_word);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::lsr32, 0b01_word);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::asr32, 0b10_word);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::rrx,   0b11_word);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rslsl, 0b00_word);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rslsr, 0b01_word);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rsasr, 0b10_word);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rsror, 0b11_word);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::lsl,   0b00_word);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::lsr,   0b01_word);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::asr,   0b10_word);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::ror,   0b11_word);
    base.opcode[25] = 1_bit;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_and_like(arm_lut& lut, arm_instruction_info base, word const id)
    -> void {
    base.opcode[24, 21] = id;
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::on);
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::off);
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_tst_like(arm_lut& lut, arm_instruction_info base, word const id)
    -> void {
    base.opcode[24, 21] = id;
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::on);
}

consteval auto qual::asign_arm_data_processing(arm_lut &lut)
    -> void {
    auto base = arm_instruction_info{
        .mask   = 0b0000'1111'1111'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'0000'0000'0000'0000'0000'0000'0000_word,
    };
    qual::arm_create_and_like<and_>(lut, base, 0b0000_word);
    qual::arm_create_and_like<eor> (lut, base, 0b0001_word);
    qual::arm_create_and_like<sub> (lut, base, 0b0010_word);
    qual::arm_create_and_like<rsb> (lut, base, 0b0011_word);
    qual::arm_create_and_like<add> (lut, base, 0b0100_word);
    qual::arm_create_and_like<adc> (lut, base, 0b0101_word);
    qual::arm_create_and_like<sbc> (lut, base, 0b0110_word);
    qual::arm_create_and_like<rsc> (lut, base, 0b0111_word);
    qual::arm_create_tst_like<tst> (lut, base, 0b1000_word);
    qual::arm_create_tst_like<teq> (lut, base, 0b1001_word);
    qual::arm_create_tst_like<cmp> (lut, base, 0b1010_word);
    qual::arm_create_tst_like<cmn> (lut, base, 0b1011_word);
    qual::arm_create_and_like<orr> (lut, base, 0b1100_word);
    qual::arm_create_and_like<mov> (lut, base, 0b1101_word);
    qual::arm_create_and_like<bic> (lut, base, 0b1110_word);
    qual::arm_create_and_like<mvn> (lut, base, 0b1111_word);
}

consteval auto qual::arm_create_mrs(arm_lut& lut)
    -> void {
    auto base = arm_instruction_info{
        .mask   = 0b0000'11111'1'111111'0000'1111'1111'1111_word,
        .opcode = 0b0000'00010'0'001111'0000'0000'0000'0000_word,
    };
    lut[cpu::arm_instruction::construct<mrs>(cpu::which_psr::cpsr).as_index()] = base;
    base.opcode[22] = 1_bit;
    lut[cpu::arm_instruction::construct<mrs>(cpu::which_psr::spsr).as_index()] = base;
}

consteval auto qual::arm_create_msr(arm_lut& lut)
    -> void {
    qual::arm_create_msr_full(lut);
    qual::arm_create_msr_flag_bits(lut);
}

consteval auto qual::arm_create_msr_full(arm_lut& lut)
    -> void {
    auto base = arm_instruction_info{
        .mask   = 0b0000'11111'1'1111111111'11111111'0000_word,
        .opcode = 0b0000'00010'0'1010011111'00000000'0000_word,
    };
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::off, 
        cpu::mask::off, 
        cpu::which_psr::cpsr
    ).as_index()] = base;
    base.opcode[22] = 1_bit;
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::off, 
        cpu::mask::off, 
        cpu::which_psr::spsr
    ).as_index()] = base;
}

consteval auto qual::arm_create_msr_flag_bits(arm_lut& lut)
    -> void {
    auto base = arm_instruction_info{
        .mask   = 0b0000'11'1'11'1'1111111111'0000'0000'0000_word,
        .opcode = 0b0000'00'0'10'0'1010001111'0000'0000'0000_word,
    };
    base.opcode[25] = 1_bit;
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::on, 
        cpu::mask::on, 
        cpu::which_psr::cpsr
    ).as_index()] = base;
    base.opcode[22] = 1_bit;
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::on, 
        cpu::mask::on, 
        cpu::which_psr::spsr
    ).as_index()] = base;

    base.opcode[25]  = 0_bit;
    base.mask[11, 4] = 0b1111'1111_word;
    base.opcode[22]  = 0_bit;
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::off, 
        cpu::mask::on, 
        cpu::which_psr::cpsr
    ).as_index()] = base;
    base.opcode[22] = 1_bit;
    lut[cpu::arm_instruction::construct<msr>(
        cpu::immediate_operand::off, 
        cpu::mask::on, 
        cpu::which_psr::spsr
    ).as_index()] = base;
    
}


consteval auto qual::asign_arm_psr_transfer(arm_lut& lut) 
    -> void {
    qual::arm_create_mrs(lut);
    qual::arm_create_msr(lut);
}

consteval auto qual::generate_arm_opcode_lut()
    -> arm_lut {
    arm_lut ret{};
    
    qual::asign_arm_branch_opcodes(ret);
    qual::asign_arm_data_processing(ret);
    qual::asign_arm_psr_transfer(ret);

    return ret;

}

inline constexpr auto arm_instruction_info_lut = qual::generate_arm_opcode_lut();
} // namespace 



auto cpu::decode_arm(u32 const instruction) noexcept -> cpu::arm_instruction {
    std::size_t index = 0;
    for(; index < arm_instruction_info_lut.size(); ++index) {
        auto const [mask, opcode] = arm_instruction_info_lut[index];
        if ((word{instruction} & mask) == opcode) break;
    }
    return cpu::arm_instruction{index};
}


} // namespace fgba
