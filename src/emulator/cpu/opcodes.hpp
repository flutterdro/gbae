#ifndef OP_CODES_HPP_
#define OP_CODES_HPP_

#include "cpudefines.hpp"

namespace fgba::cpu {
struct arm_instruction {
    u32 mask;
    u32 opcode;
};
struct thumb_instruction {
    
};
#define GEN_SPECIFIC_SHIFT(instruction_base, shift_name, shift_mask_exclude, shift_code) do {\
    ret[instruction_base ## shift_name] = {\
        .mask   = ret[instruction_base].mask   ^ (shift_mask_exclude << 4),\
        .opcode = ret[instruction_base].opcode | (shift_code << 4),\
    };\
} while (false)
#define GEN_SHIFT_VARIATIONS(instruction_base) do { \
    GEN_SPECIFIC_SHIFT(instruction_base, lsr32, 0b00000'00'0, 0b0000'0'01'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, asr32, 0b00000'00'0, 0b0000'0'10'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, rrx,   0b00000'00'0, 0b0000'0'11'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, lsl,   0b11111'00'0, 0b0000'0'00'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, lsr,   0b11111'00'0, 0b0000'0'01'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, asr,   0b11111'00'0, 0b0000'0'10'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, ror,   0b11111'00'0, 0b0000'0'11'0);\
    GEN_SPECIFIC_SHIFT(instruction_base, rslsl, 0b11110'00'0, 0b0000'1'00'1);\
    GEN_SPECIFIC_SHIFT(instruction_base, rslsr, 0b11110'00'0, 0b0000'1'01'1);\
    GEN_SPECIFIC_SHIFT(instruction_base, rsasr, 0b11110'00'0, 0b0000'1'10'1);\
    GEN_SPECIFIC_SHIFT(instruction_base, rsror, 0b11110'00'0, 0b0000'1'11'1);\
} while (false)
constexpr auto arm_instruction_init() {
    std::array<arm_instruction, arm_instruction_set::undefined> ret;
    ret[arm_instruction_set::bx] = {
        .mask   = 0x0f'ff'ff'f0,
        .opcode = 0x01'2f'ff'10,
    };
    ret[arm_instruction_set::b] = {
        .mask   = 0x0f'00'00'00,
        .opcode = 0x0a'00'00'00,
    };
    ret[arm_instruction_set::bl] = {
        .mask   = 0x0f'00'00'00,
        .opcode = 0x0b'00'00'00,
    };
    ret[arm_instruction_set::tst] = {
        .mask   = 0x0f'ff'0f'f0,
        .opcode = 0x0f'11'00'00,
    };
    GEN_SHIFT_VARIATIONS(arm_instruction_set::tst);
    ret[arm_instruction_set::tsti] = {
        .mask   = 0x0f'ff'00'00,
        .opcode = 0x0f'11'00'00,
    };
    ret[arm_instruction_set::mov] = {
        .mask   = 0x0,
        .opcode = 0x0,
    };
    GEN_SHIFT_VARIATIONS(mov);
    ret[arm_instruction_set::movs] = {
        .mask   = 0x0,
        .opcode = 0x0,
    };
    GEN_SHIFT_VARIATIONS(movs);
    ret[arm_instruction_set::movns] = {
        .mask   = 0x0,
        .opcode = 0x0,
    };
    GEN_SHIFT_VARIATIONS(movns);
};
#undef GEN_SHIFT_VARIATIONS

}



#endif