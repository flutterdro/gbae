#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpudefines.hpp"
#include <algorithm>
#include <array>
#include <ranges>
#include <utility>

namespace fgba {

namespace cpu::arm {


    auto to_index = [](instruction const instruction) {
        auto result = word{0};
        result[3, 0]  = instruction[7, 4];
        result[4]     = instruction[11, 8] != 0_word;
        result[12, 5] = instruction[27, 20];
        return result.as<std::size_t>();
    };

constexpr auto equal(std::equality_comparable auto&& val) {
    return [=](auto&& other) -> bool {
        return other == val;
    };
}
constexpr auto decode_slow(instruction const instr) {
    using enum instruction_spec::set;
    auto const p = instr[24] == 1_bit ? indexing::pre : indexing::post;
    auto const u = instr[23] == 1_bit ? direction::up : direction::down;
    auto const w = instr[21] == 1_bit ? write_back::on: write_back::off;
    auto determine_shift = [](instruction const instr) {
        if (instr[4] == 1_bit) {
            switch (instr[6, 5].value) {
                case 0b00: return shifts::rslsl;
                case 0b01: return shifts::rslsr;
                case 0b10: return shifts::rsasr;
                case 0b11: return shifts::rsror;
                default: std::unreachable();
            }
        } else if (instr[11, 7] == 0_word) {
            switch (instr[6, 5].value) {
                case 0b00: return shifts::null;
                case 0b01: return shifts::lsr32;
                case 0b10: return shifts::asr32;
                case 0b11: return shifts::rrx;
                default: std::unreachable();
            }
        } else {
            switch (instr[6, 5].value) {
                case 0b00: return shifts::lsl;
                case 0b01: return shifts::lsr;
                case 0b10: return shifts::asr;
                case 0b11: return shifts::ror;
                default: std::unreachable();
            }

        }
    };
    switch(instr[27, 26].value) {
        case 0b00: {
            auto const opcode = instr[24, 21];
            auto const tst_opcodes = std::array{0b1000_word, 0b1001_word, 0b1010_word, 0b1011_word};
            auto const i = instr[25] == 1_bit ? immediate_operand::on : immediate_operand::off;
            auto const s = instr[20] == 1_bit ? s_bit::on : s_bit::off;
            auto shift = shifts::null;
            // stoopid prohibition in constexpr context
            // if (i == immediate_operand::on) goto data_proc;
            auto const skip = i == immediate_operand::on;
            if (not skip and instr[7, 4] == 0b0001_word) {
                if (s == s_bit::off and opcode == 0b1001_word) {
                    return instruction_spec::construct<bx>();
                }
            }
            if (not skip and instr[7] == 1_bit and instr[4] == 1_bit) {
                if (instr[6, 5] == 0b00_word) {
                    auto const a = instr[21] == 1_bit ? accumulate::on : accumulate::off;
                    if (instr[23] == 1_bit) {
                        auto const sign = instr[22] == 1_bit ? mll_signedndesd::signed_ : mll_signedndesd::unsigned_;
                        return instruction_spec::construct<mll>(s, a, sign);
                    } else if (instr[24] == 1_bit) {
                        auto const b = instr[22] == 1_bit ? data_size::byte : data_size::word;
                        return instruction_spec::construct<swp>(b);
                    } else {
                        return instruction_spec::construct<mul>(s, a);
                    }
                } else {
                    auto const sign = instr[6]  == 1_bit ? mll_signedndesd::signed_ : mll_signedndesd::unsigned_;
                    auto const h    = instr[5]  == 1_bit ? data_size::hword : data_size::byte;
                    auto const i2   = instr[22] == 1_bit ? immediate_operand::on : immediate_operand::off;
                    if (instr[20] == 1_bit) {
                        return instruction_spec::construct<ldr>(
                            i2, shifts::null, u, p, w, h, sign
                        );
                    } else {
                        return instruction_spec::construct<str>(
                            i2, shifts::null, u, p, w, h
                        );
                    }
                }
            }
            shift = determine_shift(instr);
            data_proc: {
                if (s == s_bit::off and std::ranges::any_of(tst_opcodes, equal(opcode))) {
                    auto const psr = instr[22] == 1_bit ? which_psr::spsr : which_psr::cpsr;
                    if (instr[21] == 1_bit) {
                        return instruction_spec::construct<msr>(i, psr);
                    } else {
                        return instruction_spec::construct<mrs>(psr);
                    }
                } else switch (opcode.value) {
                    case 0b0000: return instruction_spec::construct<and_>(i, shift, s);
                    case 0b0001: return instruction_spec::construct<eor>(i, shift, s);
                    case 0b0010: return instruction_spec::construct<sub>(i, shift, s);
                    case 0b0011: return instruction_spec::construct<rsb>(i, shift, s);
                    case 0b0100: return instruction_spec::construct<add>(i, shift, s);
                    case 0b0101: return instruction_spec::construct<adc>(i, shift, s);
                    case 0b0110: return instruction_spec::construct<sbc>(i, shift, s);
                    case 0b0111: return instruction_spec::construct<rsc>(i, shift, s);
                    case 0b1000: return instruction_spec::construct<tst>(i, shift);
                    case 0b1001: return instruction_spec::construct<teq>(i, shift);
                    case 0b1010: return instruction_spec::construct<cmp>(i, shift);
                    case 0b1011: return instruction_spec::construct<cmn>(i, shift);
                    case 0b1100: return instruction_spec::construct<orr>(i, shift, s);
                    case 0b1101: return instruction_spec::construct<mov>(i, shift, s);
                    case 0b1110: return instruction_spec::construct<bic>(i, shift, s);
                    case 0b1111: return instruction_spec::construct<mvn>(i, shift, s);
                    default: std::unreachable();
                }
            }
        }
        case 0b01: {
            auto const b = instr[22] == 1_bit ? data_size::byte : data_size::word;
            if (instr[25] == 0_bit) {
                if (instr[20] == 1_bit) {
                    return instruction_spec::construct<ldr>(
                        immediate_operand::on, shifts::null, u, p, w, b,
                        mll_signedndesd::unsigned_
                    );
                } else {
                    return instruction_spec::construct<str>(
                        immediate_operand::on, shifts::null, u, p, w, b
                    );
                }
            } else {
                if (instr[4] == 1_bit) {
                    return instruction_spec::construct<undefined>();
                } else {
                    if (instr[20] == 1_bit) {
                        return instruction_spec::construct<ldr>(
                            immediate_operand::off, determine_shift(instr), u, p, w, b, 
                            mll_signedndesd::unsigned_
                        );
                    } else {
                        return instruction_spec::construct<str>(
                            immediate_operand::off, determine_shift(instr), u, p, w, b 
                        );
                    }
                }
            }
        }
        case 0b10: {
            if (instr[25] == 1_bit) {
                return instr[24] == 1_bit ? 
                    instruction_spec::construct<bl>() :
                    instruction_spec::construct<b>();
            } else {
                auto const s = instr[22] == 1_bit ? s_bit::on : s_bit::off;
                if (instr[20] == 1_bit) {
                    return instruction_spec::construct<ldm>(s, u, w, p);
                } else {
                    return instruction_spec::construct<stm>(s, u, w, p);
                }
            }
        }
        case 0b11: {
            if (instr[24] == 1_bit) {
                return instruction_spec::construct<swi>();
            } else {
                return instruction_spec::construct<coprocessor_placeholder>();
            }
        }
        default: std::unreachable();
    }
}



consteval auto generate_offset_lut() {
    auto ret = std::array<u16, 0b1111'1111'1111'1>{};
    for (std::size_t i = 0; i < ret.size(); ++i) {
        auto const manip = word{static_cast<u32>(i)};
        auto instr = word{0};
        instr[7, 4] = manip[3, 0];
        instr[27, 20] = manip[12, 5];
        instr[11, 8]  = manip[4] ? ~0_word : 0_word;
        ret[i] = static_cast<u16>(decode_slow({instr}).as_index()); 
    }
    return ret;
}

constexpr auto const offset_lut = generate_offset_lut();
} // namespace 


namespace cpu {
auto decode(arm::instruction const instruction) noexcept -> arm::instruction_spec {
    return {arm::offset_lut[arm::to_index(instruction)]};
}
}
} // namespace fgba
