#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpu/opcodes.hpp"
#include <catch2/catch_test_macros.hpp>
#include "emulator/cpudefines.hpp"

using namespace fgba;

TEST_CASE("Decoding arm b, bl, and bx", "[cpu][decoding]") {
    using enum cpu::arm_instruction::set;
    u32 const instruction_b  = 0b0110'1010'1010'1000'1110'0010'1101'1001;
    u32 const instruction_bl = 0b0110'1011'1010'1000'1110'0010'1101'1001;
    u32 const instruction_bx = 0b0110'0001'0010'1111'1111'1111'0001'0110;

    auto b_decoded  = cpu::decode_arm(instruction_b);
    auto bl_decoded = cpu::decode_arm(instruction_bl);
    auto bx_decoded = cpu::decode_arm(instruction_bx);

    CHECK(b_decoded.get_base()  == b);
    CHECK(bl_decoded.get_base() == bl);
    CHECK(bx_decoded.get_base() == bx);
}

TEST_CASE("Decoding arm data processing instructions", "[cpu][decoding]") {
    using enum cpu::arm_instruction::set;
    u32 const bare_bones = 0b0110'00'000000'0010'0101'0000'0000'0000_u32;
    u32 const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_u32;
    u32 const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_u32;
    u32 const ins_and    = 0b0000'00'000000'0000'0000'0000'0000'0000_u32;
    u32 const ins_adc    = 0b0000'00'001010'0000'0000'0000'0000'0000_u32;
    u32 const ins_tst    = 0b0000'00'010001'0000'0000'0000'0000'0000_u32;
    u32 const ins_cmp    = 0b0000'00'010101'0000'0000'0000'0000'0000_u32;
    u32 const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_u32;
    u32 const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_u32;
    u32 const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_u32;
    u32 const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_u32;
    u32 const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_u32;
    u32 const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_u32;
 
    SECTION("and") {
        u32 instruction = bare_bones | ins_and;
        SECTION("i and s flags") {
            auto const none_result = cpu::decode_arm(instruction);
            auto const i_on_result = cpu::decode_arm(instruction | i);
            auto const s_on_result = cpu::decode_arm(instruction | s);
            auto const both_result = cpu::decode_arm(instruction | i | s);

            auto const none_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::off
            ); 
            auto const i_on_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::on, 
                cpu::shifts::null, 
                cpu::s_bit::off
            );
            auto const s_on_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::on
            );
            auto const both_expected = cpu::arm_instruction::construct<and_>(
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

            auto const null_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::off
            );
            auto const lsl_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsl, 
                cpu::s_bit::off
            );

            auto const rslsl_expected = cpu::arm_instruction::construct<and_>(
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

            auto const lsr32_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr32, 
                cpu::s_bit::off
            );
            auto const lsr_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr, 
                cpu::s_bit::off
            );

            auto const rslsr_expected = cpu::arm_instruction::construct<and_>(
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

            auto const asr32_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr32, 
                cpu::s_bit::off
            );
            auto const asr_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr, 
                cpu::s_bit::off
            );

            auto const rsasr_expected = cpu::arm_instruction::construct<and_>(
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

            auto const rrx_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::rrx, 
                cpu::s_bit::off
            );
            auto const ror_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::ror, 
                cpu::s_bit::off
            );

            auto const rsror_expected = cpu::arm_instruction::construct<and_>(
                cpu::immediate_operand::off, 
                cpu::shifts::rsror, 
                cpu::s_bit::off
            );

            CHECK(rrx_result.as_index() == rrx_expected.as_index());
            CHECK(ror_result.as_index() == ror_expected.as_index());
            CHECK(rsror_result.as_index() == rsror_expected.as_index());
        }
    }
    SECTION("adc") {
        u32 instruction = bare_bones | ins_adc;
        SECTION("i and s flags") {
            auto const none_result = cpu::decode_arm(instruction);
            auto const i_on_result = cpu::decode_arm(instruction | i);
            auto const s_on_result = cpu::decode_arm(instruction | s);
            auto const both_result = cpu::decode_arm(instruction | i | s);

            auto const none_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::off
            ); 
            auto const i_on_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::on, 
                cpu::shifts::null, 
                cpu::s_bit::off
            );
            auto const s_on_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::on
            );
            auto const both_expected = cpu::arm_instruction::construct<adc>(
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

            auto const null_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::null, 
                cpu::s_bit::off
            );
            auto const lsl_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsl, 
                cpu::s_bit::off
            );

            auto const rslsl_expected = cpu::arm_instruction::construct<adc>(
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

            auto const lsr32_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr32, 
                cpu::s_bit::off
            );
            auto const lsr_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr, 
                cpu::s_bit::off
            );

            auto const rslsr_expected = cpu::arm_instruction::construct<adc>(
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

            auto const asr32_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr32, 
                cpu::s_bit::off
            );
            auto const asr_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr, 
                cpu::s_bit::off
            );

            auto const rsasr_expected = cpu::arm_instruction::construct<adc>(
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

            auto const rrx_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::rrx, 
                cpu::s_bit::off
            );
            auto const ror_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::ror, 
                cpu::s_bit::off
            );

            auto const rsror_expected = cpu::arm_instruction::construct<adc>(
                cpu::immediate_operand::off, 
                cpu::shifts::rsror, 
                cpu::s_bit::off
            );

            CHECK(rrx_result.as_index() == rrx_expected.as_index());
            CHECK(ror_result.as_index() == ror_expected.as_index());
            CHECK(rsror_result.as_index() == rsror_expected.as_index());
        }
    }
    SECTION("tst") {
        u32 instruction = bare_bones | ins_tst | s;
        SECTION("i and s flags") {
            auto const none_result = cpu::decode_arm(instruction);
            auto const i_on_result = cpu::decode_arm(instruction | i);
            auto const s_on_result = cpu::decode_arm(instruction | s);
            auto const both_result = cpu::decode_arm(instruction | i | s);

            auto const none_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::null 
            ); 
            auto const i_on_expected = cpu::arm_instruction::construct<tst>(
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

            auto const null_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::null 
            );
            auto const lsl_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsl 
            );

            auto const rslsl_expected = cpu::arm_instruction::construct<tst>(
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

            auto const lsr32_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr32 
            );
            auto const lsr_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr 
            );

            auto const rslsr_expected = cpu::arm_instruction::construct<tst>(
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

            auto const asr32_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr32 
            );
            auto const asr_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr 
            );

            auto const rsasr_expected = cpu::arm_instruction::construct<tst>(
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

            auto const rrx_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::rrx 
            );
            auto const ror_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::ror 
            );

            auto const rsror_expected = cpu::arm_instruction::construct<tst>(
                cpu::immediate_operand::off, 
                cpu::shifts::rsror 
            );

            CHECK(rrx_result.as_index() == rrx_expected.as_index());
            CHECK(ror_result.as_index() == ror_expected.as_index());
            CHECK(rsror_result.as_index() == rsror_expected.as_index());
        }
    }
    SECTION("cmp") {
        u32 instruction = bare_bones | ins_cmp | s;
        SECTION("i and s flags") {
            auto const none_result = cpu::decode_arm(instruction);
            auto const i_on_result = cpu::decode_arm(instruction | i);
            auto const s_on_result = cpu::decode_arm(instruction | s);
            auto const both_result = cpu::decode_arm(instruction | i | s);

            auto const none_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::null 
            ); 
            auto const i_on_expected = cpu::arm_instruction::construct<cmp>(
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

            auto const null_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::null 
            );
            auto const lsl_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsl 
            );

            auto const rslsl_expected = cpu::arm_instruction::construct<cmp>(
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

            auto const lsr32_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr32 
            );
            auto const lsr_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::lsr 
            );

            auto const rslsr_expected = cpu::arm_instruction::construct<cmp>(
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

            auto const asr32_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr32 
            );
            auto const asr_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::asr 
            );

            auto const rsasr_expected = cpu::arm_instruction::construct<cmp>(
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

            auto const rrx_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::rrx 
            );
            auto const ror_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::ror 
            );

            auto const rsror_expected = cpu::arm_instruction::construct<cmp>(
                cpu::immediate_operand::off, 
                cpu::shifts::rsror 
            );

            CHECK(rrx_result.as_index() == rrx_expected.as_index());
            CHECK(ror_result.as_index() == ror_expected.as_index());
            CHECK(rsror_result.as_index() == rsror_expected.as_index());
        }
    }

}
