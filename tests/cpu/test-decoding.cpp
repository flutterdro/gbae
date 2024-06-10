#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpu/opcodes.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <utility>
#include "emulator/cpudefines.hpp"

using namespace fgba;
using enum cpu::arm_instruction::set;

TEST_CASE("Decoding arm b, bl, and bx", "[cpu][decoding]") {
    u32 const instruction_b  = 0b0110'1010'1010'1000'1110'0010'1101'1001;
    u32 const instruction_bl = 0b0110'1011'1010'1000'1110'0010'1101'1001;
    u32 const instruction_bx = 0b0110'0001'0010'1111'1111'1111'0001'0110;

    auto b_decoded  = cpu::decode_arm(instruction_b);
    auto bl_decoded = cpu::decode_arm(instruction_bl);
    auto bx_decoded = cpu::decode_arm(instruction_bx);

    CHECK(b_decoded.base()  == b);
    CHECK(bl_decoded.base() == bl);
    CHECK(bx_decoded.base() == bx);
}

consteval auto lookup_data_proccessing_id(cpu::arm_instruction::set base) 
    -> u32 {
    switch (base) {
        case and_:return 0b0000'00'0'0000'0'0000'0000'0000'0000'0000_u32;
        case orr: return 0b0000'00'0'1100'0'0000'0000'0000'0000'0000_u32;
        case eor: return 0b0000'00'0'0001'0'0000'0000'0000'0000'0000_u32;
        case bic: return 0b0000'00'0'1110'0'0000'0000'0000'0000'0000_u32;
        case add: return 0b0000'00'0'0100'0'0000'0000'0000'0000'0000_u32;
        case adc: return 0b0000'00'0'0101'0'0000'0000'0000'0000'0000_u32;
        case sub: return 0b0000'00'0'0010'0'0000'0000'0000'0000'0000_u32;
        case sbc: return 0b0000'00'0'0110'0'0000'0000'0000'0000'0000_u32;
        case rsb: return 0b0000'00'0'0011'0'0000'0000'0000'0000'0000_u32;
        case rsc: return 0b0000'00'0'0111'0'0000'0000'0000'0000'0000_u32;
        case mov: return 0b0000'00'0'1101'0'0000'0000'0000'0000'0000_u32;
        case mvn: return 0b0000'00'0'1111'0'0000'0000'0000'0000'0000_u32;
        case tst: return 0b0000'00'0'1000'0'0000'0000'0000'0000'0000_u32;
        case teq: return 0b0000'00'0'1001'0'0000'0000'0000'0000'0000_u32;
        case cmp: return 0b0000'00'0'1010'0'0000'0000'0000'0000'0000_u32;
        case cmn: return 0b0000'00'0'1011'0'0000'0000'0000'0000'0000_u32;
        default: std::unreachable(); break;
    }
}

TEMPLATE_TEST_CASE_SIG("Decoding arm and-like data processing", "[cpu][decoding]",
    ((cpu::arm_instruction::set Instr), Instr), 
    (and_), (eor), (orr), (bic), (add), (adc), (sub), (sbc), (rsb), (rsc), (mov), (mvn)
) {
    u32 const bare_bones = 0b0110'00'000000'0010'0101'0000'0000'0000_u32;
    u32 const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_u32;
    u32 const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_u32;
    u32 const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_u32;
    u32 const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_u32;
    u32 const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_u32;
    u32 const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_u32;
    u32 const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_u32;
    u32 const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_u32;

    u32 instruction = bare_bones | lookup_data_proccessing_id(Instr);
    SECTION("i and s flags") {
        auto const none_result = cpu::decode_arm(instruction);
        auto const i_on_result = cpu::decode_arm(instruction | i);
        auto const s_on_result = cpu::decode_arm(instruction | s);
        auto const both_result = cpu::decode_arm(instruction | i | s);

        auto const none_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::null, 
            cpu::s_bit::off
        ); 
        auto const i_on_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::on, 
            cpu::shifts::null, 
            cpu::s_bit::off
        );
        auto const s_on_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::null, 
            cpu::s_bit::on
        );
        auto const both_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::on, 
            cpu::shifts::null, 
            cpu::s_bit::on
        );

        CHECK(none_result.as_index() == none_expected.as_index());
        CHECK(i_on_result.as_index() == i_on_expected.as_index());
        CHECK(s_on_result.as_index() == s_on_expected.as_index());
        CHECK(both_result.as_index() == both_expected.as_index());
    }
    SECTION("lsl") {
        instruction |= lsl;
        auto const null_result  = cpu::decode_arm(instruction);
        auto const lsl_result   = cpu::decode_arm(instruction | shift_num);
        auto const rslsl_result = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const null_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::null, 
            cpu::s_bit::off
        );
        auto const lsl_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsl, 
            cpu::s_bit::off
        );

        auto const rslsl_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rslsl, 
            cpu::s_bit::off
        );

        CHECK(null_result.as_index() == null_expected.as_index());
        CHECK(lsl_result.as_index() == lsl_expected.as_index());
        CHECK(rslsl_result.as_index() == rslsl_expected.as_index());
    }

    SECTION("lsr") {
        instruction |= lsr;
        auto const lsr32_result  = cpu::decode_arm(instruction);
        auto const lsr_result    = cpu::decode_arm(instruction | shift_num);
        auto const rslsr_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const lsr32_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsr32, 
            cpu::s_bit::off
        );
        auto const lsr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsr, 
            cpu::s_bit::off
        );

        auto const rslsr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rslsr, 
            cpu::s_bit::off
        );

        CHECK(lsr32_result.as_index() == lsr32_expected.as_index());
        CHECK(lsr_result.as_index() == lsr_expected.as_index());
        CHECK(rslsr_result.as_index() == rslsr_expected.as_index());
    }

    SECTION("asr") {
        instruction |= asr;
        auto const asr32_result  = cpu::decode_arm(instruction);
        auto const asr_result    = cpu::decode_arm(instruction | shift_num);
        auto const rsasr_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const asr32_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::asr32, 
            cpu::s_bit::off
        );
        auto const asr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::asr, 
            cpu::s_bit::off
        );

        auto const rsasr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rsasr, 
            cpu::s_bit::off
        );

        CHECK(asr32_result.as_index() == asr32_expected.as_index());
        CHECK(asr_result.as_index() == asr_expected.as_index());
        CHECK(rsasr_result.as_index() == rsasr_expected.as_index());
    }
    SECTION("ror") {
        instruction |= ror;
        auto const rrx_result  = cpu::decode_arm(instruction);
        auto const ror_result    = cpu::decode_arm(instruction | shift_num);
        auto const rsror_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const rrx_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rrx, 
            cpu::s_bit::off
        );
        auto const ror_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::ror, 
            cpu::s_bit::off
        );

        auto const rsror_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rsror, 
            cpu::s_bit::off
        );

        CHECK(rrx_result.as_index() == rrx_expected.as_index());
        CHECK(ror_result.as_index() == ror_expected.as_index());
        CHECK(rsror_result.as_index() == rsror_expected.as_index());
    }
}
TEMPLATE_TEST_CASE_SIG("Decoding arm tst-like data processing", "[cpu][decoding]",
    ((cpu::arm_instruction::set Instr), Instr), 
    (tst), (teq), (cmp), (cmn)
) {
    u32 const bare_bones = 0b0110'00'000000'0010'0101'0000'0000'0000_u32;
    u32 const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_u32;
    u32 const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_u32;
    u32 const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_u32;
    u32 const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_u32;
    u32 const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_u32;
    u32 const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_u32;
    u32 const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_u32;
    u32 const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_u32;

    u32 instruction = bare_bones | lookup_data_proccessing_id(Instr) | s;

    SECTION("i flag") {
        auto const none_result = cpu::decode_arm(instruction);
        auto const i_on_result = cpu::decode_arm(instruction | i);

        auto const none_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::null 
        ); 
        auto const i_on_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::on, 
            cpu::shifts::null 
        );

        CHECK(none_result.as_index() == none_expected.as_index());
        CHECK(i_on_result.as_index() == i_on_expected.as_index());
    }

    SECTION("lsl") {
        instruction |= lsl;
        auto const null_result  = cpu::decode_arm(instruction);
        auto const lsl_result   = cpu::decode_arm(instruction | shift_num);
        auto const rslsl_result = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const null_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::null 
        );
        auto const lsl_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsl 
        );

        auto const rslsl_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rslsl 
        );

        CHECK(null_result.as_index() == null_expected.as_index());
        CHECK(lsl_result.as_index() == lsl_expected.as_index());
        CHECK(rslsl_result.as_index() == rslsl_expected.as_index());
    }

    SECTION("lsr") {
        instruction |= lsr;
        auto const lsr32_result  = cpu::decode_arm(instruction);
        auto const lsr_result    = cpu::decode_arm(instruction | shift_num);
        auto const rslsr_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const lsr32_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsr32 
        );
        auto const lsr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::lsr 
        );

        auto const rslsr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rslsr 
        );

        CHECK(lsr32_result.as_index() == lsr32_expected.as_index());
        CHECK(lsr_result.as_index() == lsr_expected.as_index());
        CHECK(rslsr_result.as_index() == rslsr_expected.as_index());
    }

    SECTION("asr") {
        instruction |= asr;
        auto const asr32_result  = cpu::decode_arm(instruction);
        auto const asr_result    = cpu::decode_arm(instruction | shift_num);
        auto const rsasr_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const asr32_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::asr32 
        );
        auto const asr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::asr 
        );

        auto const rsasr_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rsasr 
        );

        CHECK(asr32_result.as_index() == asr32_expected.as_index());
        CHECK(asr_result.as_index() == asr_expected.as_index());
        CHECK(rsasr_result.as_index() == rsasr_expected.as_index());
    }

    SECTION("ror") {
        instruction |= ror;
        auto const rrx_result  = cpu::decode_arm(instruction);
        auto const ror_result    = cpu::decode_arm(instruction | shift_num);
        auto const rsror_result  = cpu::decode_arm(instruction | reg_shift | shift_num);

        auto const rrx_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rrx 
        );
        auto const ror_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::ror 
        );

        auto const rsror_expected = cpu::arm_instruction::construct<Instr>(
            cpu::immediate_operand::off, 
            cpu::shifts::rsror 
        );

        CHECK(rrx_result.as_index() == rrx_expected.as_index());
        CHECK(ror_result.as_index() == ror_expected.as_index());
        CHECK(rsror_result.as_index() == rsror_expected.as_index());
    }

}

TEST_CASE("Decoding arm msr", "[cpu][decoding][arm]") {
    auto const skeleton = 0b0100'00'0'10'0'10100'0'1111'00000000'1011_word;
    auto const spsr     = 0b0000'00'0'00'1'00000'0'0000'00000000'0000_word;
    auto const imop     = 0b0000'00'1'00'0'00000'0'0000'00000000'0000_word;
    auto const whole    = 0b0000'00'0'00'0'00000'1'0000'00000000'0000_word;
    SECTION("whole psr transfer") {
        auto const cpsr_result   = cpu::decode_arm((skeleton | whole).value);
        auto const cpsr_expected = cpu::arm_instruction::construct<msr>(
            cpu::immediate_operand::off,
            cpu::mask::off,
            cpu::which_psr::cpsr
        );
        auto const spsr_result   = cpu::decode_arm((skeleton | whole | spsr).value);
        auto const spsr_expected = cpu::arm_instruction::construct<msr>(
            cpu::immediate_operand::off,
            cpu::mask::off,
            cpu::which_psr::spsr
        );

        CHECK(cpsr_result.as_index() == cpsr_expected.as_index());
        CHECK(spsr_result.as_index() == spsr_expected.as_index());
    }
    SECTION("flag bits only") {
        SECTION("immediate_operand") {
            auto const imop_result   = cpu::decode_arm((skeleton | imop).value);
            auto const imop_expected = cpu::arm_instruction::construct<msr>(
                cpu::immediate_operand::on,
                cpu::mask::on,
                cpu::which_psr::cpsr
            );
            auto const noimop_result   = cpu::decode_arm((skeleton).value);
            auto const noimop_expected = cpu::arm_instruction::construct<msr>(
                cpu::immediate_operand::off,
                cpu::mask::on,
                cpu::which_psr::cpsr
            );
        }
        SECTION("cpsr and spsr") {
            auto const cpsr_result   = cpu::decode_arm((skeleton).value);
            auto const cpsr_expected = cpu::arm_instruction::construct<msr>(
                cpu::immediate_operand::off,
                cpu::mask::on,
                cpu::which_psr::cpsr
            );
            auto const spsr_result   = cpu::decode_arm((skeleton | spsr).value);
            auto const spsr_expected = cpu::arm_instruction::construct<msr>(
                cpu::immediate_operand::off,
                cpu::mask::on,
                cpu::which_psr::spsr
            );
            CHECK(cpsr_result.as_index() == cpsr_expected.as_index());
            CHECK(spsr_result.as_index() == spsr_expected.as_index());
        }
    }
}

TEST_CASE("Decoding arm mrs", "[cpu][decoding][arm]") {
    auto const skeleton  = 0b0100'00010'0'001111'0101'0000'0000'0000_u32;
    auto const spsr      = 0b0000'00000'1'000000'0000'0000'0000'0000_u32;
    auto instruction     = 0_u32;
    SECTION("cpsr transfer") {
        auto const result   = cpu::decode_arm(skeleton);
        auto const expected = cpu::arm_instruction::construct<mrs>(cpu::which_psr::cpsr);

        CHECK(result.as_index() == expected.as_index());
    }
    SECTION("spsr transfer") {
        auto const result   = cpu::decode_arm(skeleton | spsr);
        auto const expected = cpu::arm_instruction::construct<mrs>(cpu::which_psr::spsr);
    }
}
