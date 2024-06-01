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
    u32 mask;
    u32 opcode;
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
consteval auto arm_create_and_like(arm_lut&, arm_instruction_info, u32 id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_tst_like(arm_lut&, arm_instruction_info, u32 id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_s_bit_variation(arm_lut&, arm_instruction_info, cpu::s_bit)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_special_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, u32 id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_register_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, u32 id)
    -> void;
template<cpu::arm_instruction::set>
consteval auto arm_create_normal_shift_variation(arm_lut&, arm_instruction_info, cpu::s_bit, cpu::shifts, u32 id)
    -> void;
} // namespace qual

consteval auto qual::asign_arm_branch_opcodes(arm_lut& lut)
    -> void {
    lut[cpu::arm_instruction::construct<b>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_u32,
        .opcode = 0b0000'1010'0000'0000'0000'0000'0000'0000_u32,
    };
    lut[cpu::arm_instruction::construct<bl>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_u32,
        .opcode = 0b0000'1011'0000'0000'0000'0000'0000'0000_u32,
    };
    lut[cpu::arm_instruction::construct<bx>().as_index()] = {
        .mask   = 0b0000'1111'1111'1111'1111'1111'1111'0000_u32,
        .opcode = 0b0000'0001'0010'1111'1111'1111'0001'0000_u32,
    };
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_special_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, u32 id)
    -> void {
    base.mask   |= 0b1111'1111_u32 << 4_u32;
    base.opcode |= id << 5_u32;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_register_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, u32 id)
    -> void {
    base.mask   |= 0b1111_u32 << 4_u32;
    base.opcode |= id << 5_u32;
    base.opcode |= 1_u32 << 4_u32;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_normal_shift_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s, cpu::shifts shift, u32 id)
    -> void {
    base.mask   |= 0b111_u32 << 4_u32;
    base.opcode |= id << 5_u32;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_s_bit_variation(arm_lut& lut, arm_instruction_info base, cpu::s_bit s)
    -> void {
    auto s_bit = u32{s == cpu::s_bit::on};
    base.opcode |= s_bit << 20_u32;
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::null,  0b00_u32);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::lsr32, 0b01_u32);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::asr32, 0b10_u32);
    qual::arm_create_special_shift_variation <InstructionBase>(lut, base, s, cpu::shifts::rrx,   0b11_u32);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rslsl, 0b00_u32);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rslsr, 0b01_u32);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rsasr, 0b10_u32);
    qual::arm_create_register_shift_variation<InstructionBase>(lut, base, s, cpu::shifts::rsror, 0b11_u32);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::lsl,   0b00_u32);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::lsr,   0b01_u32);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::asr,   0b10_u32);
    qual::arm_create_normal_shift_variation  <InstructionBase>(lut, base, s, cpu::shifts::ror,   0b11_u32);
    base.opcode |= 1_u32 << 25_u32;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null, s).as_index()] = base;
    } else {
        lut[cpu::arm_instruction::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null).as_index()] = base;
    }
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_and_like(arm_lut& lut, arm_instruction_info base, u32 id)
    -> void {
    base.opcode |= id << 21_u32;
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::on);
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::off);
}
template<cpu::arm_instruction::set InstructionBase>
consteval auto qual::arm_create_tst_like(arm_lut& lut, arm_instruction_info base, u32 id)
    -> void {
    base.opcode |= id << 21_u32;
    arm_create_s_bit_variation<InstructionBase>(lut, base, cpu::s_bit::on);
}

consteval auto qual::asign_arm_data_processing(arm_lut &lut)
    -> void {
    auto base = arm_instruction_info{
        .mask   = 0b0000'1111'1111'0000'0000'0000'0000'0000_u32,
        .opcode = 0b0000'0000'0000'0000'0000'0000'0000'0000_u32,
    };
    qual::arm_create_and_like<and_>(lut, base, 0b0000);
    qual::arm_create_and_like<eor> (lut, base, 0b0001);
    qual::arm_create_and_like<sub> (lut, base, 0b0010);
    qual::arm_create_and_like<rsb> (lut, base, 0b0011);
    qual::arm_create_and_like<add> (lut, base, 0b0100);
    qual::arm_create_and_like<adc> (lut, base, 0b0101);
    qual::arm_create_and_like<sbc> (lut, base, 0b0110);
    qual::arm_create_and_like<rsc> (lut, base, 0b0111);
    qual::arm_create_tst_like<tst> (lut, base, 0b1000);
    qual::arm_create_tst_like<teq> (lut, base, 0b1001);
    qual::arm_create_tst_like<cmp> (lut, base, 0b1010);
    qual::arm_create_tst_like<cmn> (lut, base, 0b1011);
    qual::arm_create_and_like<orr> (lut, base, 0b1100);
    qual::arm_create_and_like<mov> (lut, base, 0b1101);
    qual::arm_create_and_like<bic> (lut, base, 0b1110);
    qual::arm_create_and_like<mvn> (lut, base, 0b1111);
}



consteval auto qual::generate_arm_opcode_lut()
    -> arm_lut {
    arm_lut ret{};
    
    qual::asign_arm_branch_opcodes(ret);
    qual::asign_arm_data_processing(ret);

    return ret;

}

inline constexpr auto arm_instruction_info_lut = qual::generate_arm_opcode_lut();
} // namespace 



auto cpu::decode_arm(u32 instruction) noexcept -> cpu::arm_instruction {
    std::size_t index = 0;
    for(; index < arm_instruction_info_lut.size(); ++index) {
        auto const [mask, opcode] = arm_instruction_info_lut[index];
        if ((instruction & mask) == opcode) break;
    }
    return cpu::arm_instruction{index};
}


} // namespace fgba
