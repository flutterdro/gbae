#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpu/opcodes.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <utility>
#include "emulator/cpudefines.hpp"
using namespace fgba;
using namespace fgba::cpu;
using enum arm::instruction_spec::set;

TEST_CASE("Decoding arm b, bl, and bx", "[cpu][decoding]") {
    word const instruction_b  = 0b0110'1010'1010'1000'1110'0010'1101'1001_word;
    word const instruction_bl = 0b0110'1011'1010'1000'1110'0010'1101'1001_word;
    word const instruction_bx = 0b0110'0001'0010'1111'1111'1111'0001'0110_word;

    auto b_decoded  = decode({instruction_b});
    auto bl_decoded = decode({instruction_bl});
    auto bx_decoded = decode({instruction_bx});

    CHECK(b_decoded.base()  == b);
    CHECK(bl_decoded.base() == bl);
    CHECK(bx_decoded.base() == bx);
}

consteval auto lookup_data_proccessing_id(arm::instruction_spec::set base) 
    -> word {
    switch (base) {
        case and_:return 0b0000'00'0'0000'0'0000'0000'0000'0000'0000_word;
        case orr: return 0b0000'00'0'1100'0'0000'0000'0000'0000'0000_word;
        case eor: return 0b0000'00'0'0001'0'0000'0000'0000'0000'0000_word;
        case bic: return 0b0000'00'0'1110'0'0000'0000'0000'0000'0000_word;
        case add: return 0b0000'00'0'0100'0'0000'0000'0000'0000'0000_word;
        case adc: return 0b0000'00'0'0101'0'0000'0000'0000'0000'0000_word;
        case sub: return 0b0000'00'0'0010'0'0000'0000'0000'0000'0000_word;
        case sbc: return 0b0000'00'0'0110'0'0000'0000'0000'0000'0000_word;
        case rsb: return 0b0000'00'0'0011'0'0000'0000'0000'0000'0000_word;
        case rsc: return 0b0000'00'0'0111'0'0000'0000'0000'0000'0000_word;
        case mov: return 0b0000'00'0'1101'0'0000'0000'0000'0000'0000_word;
        case mvn: return 0b0000'00'0'1111'0'0000'0000'0000'0000'0000_word;
        case tst: return 0b0000'00'0'1000'0'0000'0000'0000'0000'0000_word;
        case teq: return 0b0000'00'0'1001'0'0000'0000'0000'0000'0000_word;
        case cmp: return 0b0000'00'0'1010'0'0000'0000'0000'0000'0000_word;
        case cmn: return 0b0000'00'0'1011'0'0000'0000'0000'0000'0000_word;
        default: std::unreachable(); break;
    }
}

TEMPLATE_TEST_CASE_SIG("Decoding arm and-like data processing", "[cpu][decoding]",
    ((arm::instruction_spec::set Instr), Instr), 
    (and_), (eor), (orr), (bic), (add), (adc), (sub), (sbc), (rsb), (rsc), (mov), (mvn)
) {
    auto const bare_bones = 0b0110'00'000000'0010'0101'0000'0000'0000_word;
    auto const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_word;
    auto const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_word;
    auto const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_word;
    auto const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_word;
    auto const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_word;
    auto const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_word;
    auto const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_word;
    auto const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_word;

    word instruction = bare_bones | lookup_data_proccessing_id(Instr);
    SECTION("i and s flags") {
        auto const none_result = decode({instruction});
        auto const i_on_result = decode({instruction | i});
        auto const s_on_result = decode({instruction | s});
        auto const both_result = decode({instruction | i | s});

        auto const none_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::null, 
            s_bit::off
        ); 
        auto const i_on_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::on, 
            shifts::null, 
            s_bit::off
        );
        auto const s_on_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::null, 
            s_bit::on
        );
        auto const both_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::on, 
            shifts::null, 
            s_bit::on
        );

        CHECK(none_result.as_index() == none_expected.as_index());
        CHECK(i_on_result.as_index() == i_on_expected.as_index());
        CHECK(s_on_result.as_index() == s_on_expected.as_index());
        CHECK(both_result.as_index() == both_expected.as_index());
    }
    SECTION("lsl") {
        instruction |= lsl;
        auto const null_result  = decode({instruction});
        auto const lsl_result   = decode({instruction | shift_num});
        auto const rslsl_result = decode({instruction | reg_shift | shift_num});

        auto const null_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::null, 
            s_bit::off
        );
        auto const lsl_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsl, 
            s_bit::off
        );

        auto const rslsl_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rslsl, 
            s_bit::off
        );

        CHECK(null_result.as_index() == null_expected.as_index());
        CHECK(lsl_result.as_index() == lsl_expected.as_index());
        CHECK(rslsl_result.as_index() == rslsl_expected.as_index());
    }

    SECTION("lsr") {
        instruction |= lsr;
        auto const lsr32_result  = decode({instruction});
        auto const lsr_result    = decode({instruction | shift_num});
        auto const rslsr_result  = decode({instruction | reg_shift | shift_num});

        auto const lsr32_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsr32, 
            s_bit::off
        );
        auto const lsr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsr, 
            s_bit::off
        );

        auto const rslsr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rslsr, 
            s_bit::off
        );

        CHECK(lsr32_result.as_index() == lsr32_expected.as_index());
        CHECK(lsr_result.as_index() == lsr_expected.as_index());
        CHECK(rslsr_result.as_index() == rslsr_expected.as_index());
    }

    SECTION("asr") {
        instruction |= asr;
        auto const asr32_result  = decode({instruction});
        auto const asr_result    = decode({instruction | shift_num});
        auto const rsasr_result  = decode({instruction | reg_shift | shift_num});

        auto const asr32_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::asr32, 
            s_bit::off
        );
        auto const asr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::asr, 
            s_bit::off
        );

        auto const rsasr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rsasr, 
            s_bit::off
        );

        CHECK(asr32_result.as_index() == asr32_expected.as_index());
        CHECK(asr_result.as_index() == asr_expected.as_index());
        CHECK(rsasr_result.as_index() == rsasr_expected.as_index());
    }
    SECTION("ror") {
        instruction |= ror;
        auto const rrx_result  = decode({instruction});
        auto const ror_result    = decode({instruction | shift_num});
        auto const rsror_result  = decode({instruction | reg_shift | shift_num});

        auto const rrx_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rrx, 
            s_bit::off
        );
        auto const ror_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::ror, 
            s_bit::off
        );

        auto const rsror_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rsror, 
            s_bit::off
        );

        CHECK(rrx_result.as_index() == rrx_expected.as_index());
        CHECK(ror_result.as_index() == ror_expected.as_index());
        CHECK(rsror_result.as_index() == rsror_expected.as_index());
    }
}
TEMPLATE_TEST_CASE_SIG("Decoding arm tst-like data processing", "[cpu][decoding]",
    ((arm::instruction_spec::set Instr), Instr), 
    (tst), (teq), (cmp), (cmn)
) {
    auto const bare_bones = 0b0110'00'000000'0010'0101'0000'0000'0000_word;
    auto const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_word;
    auto const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_word;
    auto const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_word;
    auto const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_word;
    auto const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_word;
    auto const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_word;
    auto const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_word;
    auto const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_word;

    word instruction = bare_bones | lookup_data_proccessing_id(Instr) | s;

    SECTION("i flag") {
        auto const none_result = decode({instruction});
        auto const i_on_result = decode({instruction | i});

        auto const none_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::null 
        ); 
        auto const i_on_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::on, 
            shifts::null 
        );

        CHECK(none_result.as_index() == none_expected.as_index());
        CHECK(i_on_result.as_index() == i_on_expected.as_index());
    }

    SECTION("lsl") {
        instruction |= lsl;
        auto const null_result  = decode({instruction});
        auto const lsl_result   = decode({instruction | shift_num});
        auto const rslsl_result = decode({instruction | reg_shift | shift_num});

        auto const null_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::null 
        );
        auto const lsl_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsl 
        );

        auto const rslsl_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rslsl 
        );

        CHECK(null_result.as_index() == null_expected.as_index());
        CHECK(lsl_result.as_index() == lsl_expected.as_index());
        CHECK(rslsl_result.as_index() == rslsl_expected.as_index());
    }

    SECTION("lsr") {
        instruction |= lsr;
        auto const lsr32_result  = decode({instruction});
        auto const lsr_result    = decode({instruction | shift_num});
        auto const rslsr_result  = decode({instruction | reg_shift | shift_num});

        auto const lsr32_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsr32 
        );
        auto const lsr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::lsr 
        );

        auto const rslsr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rslsr 
        );

        CHECK(lsr32_result.as_index() == lsr32_expected.as_index());
        CHECK(lsr_result.as_index() == lsr_expected.as_index());
        CHECK(rslsr_result.as_index() == rslsr_expected.as_index());
    }

    SECTION("asr") {
        instruction |= asr;
        auto const asr32_result  = decode({instruction});
        auto const asr_result    = decode({instruction | shift_num});
        auto const rsasr_result  = decode({instruction | reg_shift | shift_num});

        auto const asr32_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::asr32 
        );
        auto const asr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::asr 
        );

        auto const rsasr_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rsasr 
        );

        CHECK(asr32_result.as_index() == asr32_expected.as_index());
        CHECK(asr_result.as_index() == asr_expected.as_index());
        CHECK(rsasr_result.as_index() == rsasr_expected.as_index());
    }

    SECTION("ror") {
        instruction |= ror;
        auto const rrx_result  = decode({instruction});
        auto const ror_result    = decode({instruction | shift_num});
        auto const rsror_result  = decode({instruction | reg_shift | shift_num});

        auto const rrx_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rrx 
        );
        auto const ror_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::ror 
        );

        auto const rsror_expected = arm::instruction_spec::construct<Instr>(
            immediate_operand::off, 
            shifts::rsror 
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
        auto const cpsr_result   = decode({skeleton | whole});
        auto const cpsr_expected = arm::instruction_spec::construct<msr>(
            immediate_operand::off,
            mask::off,
            which_psr::cpsr
        );
        auto const spsr_result   = decode({skeleton | whole | spsr});
        auto const spsr_expected = arm::instruction_spec::construct<msr>(
            immediate_operand::off,
            mask::off,
            which_psr::spsr
        );

        CHECK(cpsr_result.as_index() == cpsr_expected.as_index());
        CHECK(spsr_result.as_index() == spsr_expected.as_index());
    }
    SECTION("flag bits only") {
        SECTION("immediate_operand") {
            auto const imop_result   = decode({skeleton | imop});
            auto const imop_expected = arm::instruction_spec::construct<msr>(
                immediate_operand::on,
                mask::on,
                which_psr::cpsr
            );
            auto const noimop_result   = decode({skeleton});
            auto const noimop_expected = arm::instruction_spec::construct<msr>(
                immediate_operand::off,
                mask::on,
                which_psr::cpsr
            );
            CHECK(imop_result.as_index() == imop_expected.as_index());
            CHECK(noimop_result.as_index() == noimop_expected.as_index());
        }
        SECTION("cpsr and spsr") {
            auto const cpsr_result   = decode({skeleton});
            auto const cpsr_expected = arm::instruction_spec::construct<msr>(
                immediate_operand::off,
                mask::on,
                which_psr::cpsr
            );
            auto const spsr_result   = decode({skeleton | spsr});
            auto const spsr_expected = arm::instruction_spec::construct<msr>(
                immediate_operand::off,
                mask::on,
                which_psr::spsr
            );
            CHECK(cpsr_result.as_index() == cpsr_expected.as_index());
            CHECK(spsr_result.as_index() == spsr_expected.as_index());
        }
    }
}
// static_assert(arm::instruction_spec{375}.base());

TEST_CASE("Decoding arm mrs", "[cpu][decoding][arm]") {
    auto const skeleton  = 0b0100'00010'0'001111'0101'0000'0000'0000_word;
    auto const spsr      = 0b0000'00000'1'000000'0000'0000'0000'0000_word;
    auto instruction     = 0_word;
    SECTION("cpsr transfer") {
        auto const result   = decode({skeleton});
        auto const expected = arm::instruction_spec::construct<mrs>(which_psr::cpsr);

        CHECK(result.as_index() == expected.as_index());
    }
    SECTION("spsr transfer") {
        auto const result   = decode({skeleton | spsr});
        auto const expected = arm::instruction_spec::construct<mrs>(which_psr::spsr);

        CHECK(result.as_index() == expected.as_index());
    }
}
