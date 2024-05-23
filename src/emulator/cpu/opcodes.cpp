#include "emulator/cpu/opcodes.hpp"

namespace fgba {

namespace {
struct arm_instruction_info {
    u32 mask;
    u32 opcode;
};

inline constexpr std::array<arm_instruction_info, cpu::arm_instruction::count()> arm_instruction_info_lut{};
}
auto cpu::decode_arm(u32 instruction) noexcept -> cpu::arm_instruction {
    std::size_t index = 0;
    for(; index < arm_instruction_info_lut.size(); ++index) {
        auto const [mask, opcode] = arm_instruction_info_lut[index];
        if ((instruction & mask) == opcode) break;
    }
    return cpu::arm_instruction{index};
}
}
