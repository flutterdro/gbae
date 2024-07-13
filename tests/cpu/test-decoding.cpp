#include "catch2/catch_message.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpu/opcodes.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <format>
#include <utility>
#include "emulator/cpudefines.hpp"
using namespace fgba;
using namespace fgba::cpu;
using enum arm::instruction_spec::set;

TEST_CASE("Decoding arm b, bl, and bx", "[cpu][arm][decoding]") {
    word const instruction_b  = 0b0110'1010'1010'1000'1110'0010'1101'1001_word;
    word const instruction_bl = 0b0110'1011'1010'1000'1110'0010'1101'1001_word;
    word const instruction_bx = 0b0110'0001'0010'1111'1111'1111'0001'0110_word;

    auto b_decoded  = decode({instruction_b});
    auto bl_decoded = decode({instruction_bl});
    auto bx_decoded = decode({instruction_bx});
    arm::instruction_spec::construct<b>();

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

TEMPLATE_TEST_CASE_SIG("Decoding arm and-like data processing", "[cpu][arm][decoding]",
    ((arm::instruction_spec::set Instr), Instr), 
    (and_), (eor), (orr), (bic), (add), (adc), (sub), (sbc), (rsb), (rsc), (mov), (mvn)
) {
    auto const bare_bones = 0b0110'00'000000'1111'1111'0000'0000'0000_word;
    auto const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_word;
    auto const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_word;
    auto const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_word;
    auto const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_word;
    auto const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_word;
    auto const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_word;
    auto const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_word;
    auto const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_word;

    auto shift_specifier = [=](shifts shift) {
        auto result = word{0};
        switch(shift) {
            case shifts::null:  return lsl;
            case shifts::lsr32: return lsr;
            case shifts::asr32: return asr;
            case shifts::rrx:   return ror;
            case shifts::lsl:   return lsl | shift_num;
            case shifts::lsr:   return lsr | shift_num;
            case shifts::asr:   return asr | shift_num;
            case shifts::ror:   return ror | shift_num;
            case shifts::rslsl: return lsl | shift_num | reg_shift;
            case shifts::rslsr: return lsr | shift_num | reg_shift;
            case shifts::rsasr: return asr | shift_num | reg_shift;
            case shifts::rsror: return ror | shift_num | reg_shift;
            default: std::unreachable();
        }
    };
    auto i_specifier = [=](immediate_operand i_) { return i_ == immediate_operand::on ? i : 0_word; }; //NOLINT 
    auto s_specifier = [=](s_bit s_) { return s_ == s_bit::on ? s : 0_word; }; //NOLINT 

    auto const s_var = GENERATE(s_bit::on, s_bit::off);

    auto const instruction = bare_bones | lookup_data_proccessing_id(Instr);
   
    SECTION("shifts") {
        auto const shift_var = GENERATE(
            shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx, 
            shifts::lsl, shifts::lsr, shifts::asr, shifts::ror, 
            shifts::rslsl, shifts::rslsr, shifts::rsasr, shifts::rsror
        );
        auto const expected = arm::instruction_spec::construct<Instr>(immediate_operand::off, shift_var, s_var);
        auto const result   = decode({instruction | shift_specifier(shift_var) | s_specifier(s_var)});
        CHECK(expected.as_index() == result.as_index());
    }
    SECTION("i flag") {
        auto const expected = arm::instruction_spec::construct<Instr>(immediate_operand::on, shifts::null, s_var);
        auto const result   = decode({instruction | i | s_specifier(s_var)});
        CHECK(expected.as_index() == result.as_index());
    }
}
TEMPLATE_TEST_CASE_SIG("Decoding arm tst-like data processing", "[cpu][arm][decoding]",
    ((arm::instruction_spec::set Instr), Instr), 
    (tst), (teq), (cmp), (cmn)
) {
    auto const bare_bones = 0b0110'00'000000'1111'1111'0000'0000'0000_word;
    auto const i          = 0b0000'00'100000'0000'0000'0000'0000'0000_word;
    auto const s          = 0b0000'00'000001'0000'0000'0000'0000'0000_word;
    auto const lsl        = 0b0000'00'000000'0000'0000'0000'0000'0000_word;
    auto const lsr        = 0b0000'00'000000'0000'0000'0000'0010'0000_word;
    auto const asr        = 0b0000'00'000000'0000'0000'0000'0100'0000_word;
    auto const ror        = 0b0000'00'000000'0000'0000'0000'0110'0000_word;
    auto const reg_shift  = 0b0000'00'000000'0000'0000'0000'0001'0000_word;
    auto const shift_num  = 0b0000'00'000000'0000'0000'1111'0000'0000_word;

    auto shift_specifier = [=](shifts shift) {
        auto result = word{0};
        switch(shift) {
            case shifts::null:  return lsl;
            case shifts::lsr32: return lsr;
            case shifts::asr32: return asr;
            case shifts::rrx:   return ror;
            case shifts::lsl:   return lsl | shift_num;
            case shifts::lsr:   return lsr | shift_num;
            case shifts::asr:   return asr | shift_num;
            case shifts::ror:   return ror | shift_num;
            case shifts::rslsl: return lsl | shift_num | reg_shift;
            case shifts::rslsr: return lsr | shift_num | reg_shift;
            case shifts::rsasr: return asr | shift_num | reg_shift;
            case shifts::rsror: return ror | shift_num | reg_shift;
            default: std::unreachable();
        }
    };
    auto i_specifier = [=](immediate_operand i_) { return i_ == immediate_operand::on ? i : 0_word; };  //NOLINT

    auto const instruction = bare_bones | lookup_data_proccessing_id(Instr) | s;
    
    SECTION("shifts") {
        auto const shift_var = GENERATE(
            shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx, 
            shifts::lsl, shifts::lsr, shifts::asr, shifts::ror, 
            shifts::rslsl, shifts::rslsr, shifts::rsasr, shifts::rsror
        );
        auto const expected = arm::instruction_spec::construct<Instr>(immediate_operand::off, shift_var);
        auto const result   = decode({instruction | shift_specifier(shift_var)});
        CHECK(expected.as_index() == result.as_index());
    }

   SECTION("immediate operand") {
        auto const expected = arm::instruction_spec::construct<Instr>(immediate_operand::on, shifts::null);
        auto const result   = decode({instruction | i});
        CHECK(expected.as_index() == result.as_index());
    }

}

TEST_CASE("Decoding arm msr", "[cpu][decoding][arm]") {
    auto const skeleton = 0b0100'00'0'10'0'10100'0'1111'00000000'1011_word;
    auto const spsr     = 0b0000'00'0'00'1'00000'0'0000'00000000'0000_word;
    auto const imop     = 0b0000'00'1'00'0'00000'0'0000'00000000'0000_word;
    auto const whole    = 0b0000'00'0'00'0'00000'1'0000'00000000'0000_word;

    auto const psr_specifier = [=](which_psr psr) { return psr == which_psr::spsr ? spsr : 0_word; };
    auto const i_specifier   = [=](immediate_operand i) { return i == immediate_operand::on ? imop : 0_word; };

    auto psr_var = GENERATE(which_psr::spsr, which_psr::cpsr);

    SECTION("whole psr transfer") {
        auto const result   = decode({skeleton | whole | psr_specifier(psr_var)});
        auto const expected = arm::instruction_spec::construct<msr>(
            immediate_operand::off,
            psr_var
        );
        CHECK(result.as_index() == expected.as_index());
    }
    SECTION("flag bits only") {
        auto const i_var = GENERATE(immediate_operand::on, immediate_operand::off);

        auto const result   = decode({skeleton | psr_specifier(psr_var) | i_specifier(i_var)});
        auto const expected = arm::instruction_spec::construct<msr>(
            i_var,
            psr_var
        );
        CHECK(result.as_index() == expected.as_index());
    }
}

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

TEST_CASE("Decoding arm mul", "[cpu][decoding][arm]") {
    auto const skeleton = 0b1010'000000'0'0'1111'1111'0000'1001'1011_word;
    auto const a        = 0b0000'000000'1'0'0000'0000'0000'0000'0000_word;
    auto const s        = 0b0000'000000'0'1'0000'0000'0000'0000'0000_word;

    auto a_d    = [=](accumulate a_) { return a_ == accumulate::on ? a : 0_word; };  //NOLINT
    auto s_d    = [=](s_bit s_) { return s_ == s_bit::on ? s : 0_word; };            //NOLINT

    auto s_var    = GENERATE(s_bit::on, s_bit::off);
    auto a_var    = GENERATE(accumulate::on, accumulate::off);

    auto const expected = arm::instruction_spec::construct<mul>(s_var, a_var);
    auto const result   = decode({skeleton | s_d(s_var) | a_d(a_var)});

    CHECK(result.as_index() == expected.as_index());
}
TEST_CASE("Decoding arm mll", "[cpu][decoding][arm]") {
    auto const skeleton = 0b1010'00001'0'0'0'1111'1111'0000'1001'1011_word;
    auto const a        = 0b0000'00000'0'1'0'0000'0000'0000'0000'0000_word;
    auto const s        = 0b0000'00000'0'0'1'0000'0000'0000'0000'0000_word;
    auto const sign     = 0b0000'00000'1'0'0'0000'0000'0000'0000'0000_word;
    auto a_d    = [=](accumulate a_) { return a_ == accumulate::on ? a : 0_word; };  //NOLINT
    auto s_d    = [=](s_bit s_) { return s_ == s_bit::on ? s : 0_word; };            //NOLINT
    auto sign_d = [=](mll_signedndesd sign_) { return sign_ == mll_signedndesd::signed_ ? sign : 0_word; }; //NOLINT

    auto s_var    = GENERATE(s_bit::on, s_bit::off);
    auto a_var    = GENERATE(accumulate::on, accumulate::off);
    auto sign_var = GENERATE(mll_signedndesd::signed_, mll_signedndesd::unsigned_);

    auto const expected = arm::instruction_spec::construct<mll>(s_var, a_var, sign_var);
    auto const result   = decode({skeleton | s_d(s_var) | a_d(a_var) | sign_d(sign_var)});

    CHECK(result.as_index() == expected.as_index());

}
TEST_CASE("Decoding arm ldr", "[cpu][decoding][arm]") {
    auto const skeleton1 = 0b1010'01'0'0'0'0'0'1'0000'0000'000000000000_word;
    auto const i1        = 0b0000'00'1'0'0'0'0'0'0000'0000'000000000000_word;
    auto const p         = 0b0000'00'0'1'0'0'0'0'0000'0000'000000000000_word;
    auto const u         = 0b0000'00'0'0'1'0'0'0'0000'0000'000000000000_word;
    auto const b         = 0b0000'00'0'0'0'1'0'0'0000'0000'000000000000_word;
    auto const w         = 0b0000'00'0'0'0'0'1'0'0000'0000'000000000000_word;
    auto const lsl       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0000'0000_word;
    auto const lsr       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0010'0000_word;
    auto const asr       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0100'0000_word;
    auto const ror       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0110'0000_word;
    auto const reg_shift = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0001'0000_word;
    auto const shift_num = 0b0000'00'0'0'0'0'0'0'0000'0000'1111'0000'0000_word;

    auto const skeleton2 = 0b1010'000'0'0'0'0'1'0000'0000'0000'1'0'0'1'1100_word;
    auto const i2        = 0b0000'000'0'0'1'0'0'0000'0000'0000'0'0'0'0'0000_word;
    auto const s         = 0b0000'000'0'0'0'0'0'0000'0000'0000'0'1'0'0'0000_word;
    auto const h         = 0b0000'000'0'0'0'0'0'0000'0000'0000'0'0'1'0'0000_word;

    auto shift_specifier = [=](shifts shift) {
        auto result = word{0};
        switch(shift) {
            case shifts::null:  return lsl;
            case shifts::lsr32: return lsr;
            case shifts::asr32: return asr;
            case shifts::rrx:   return ror;
            case shifts::lsl:   return lsl | shift_num;
            case shifts::lsr:   return lsr | shift_num;
            case shifts::asr:   return asr | shift_num;
            case shifts::ror:   return ror | shift_num;
            case shifts::rslsl: return lsl | shift_num | reg_shift;
            case shifts::rslsr: return lsr | shift_num | reg_shift;
            case shifts::rsasr: return asr | shift_num | reg_shift;
            case shifts::rsror: return ror | shift_num | reg_shift;
            default: std::unreachable();
        }
    };
    auto p_d = [=](indexing ind) { return ind == indexing::pre ? p : 0_word; };
    auto u_d = [=](direction dir) { return dir == direction::up ? u : 0_word; };
    auto w_d = [=](write_back wb) { return wb == write_back::on ? w : 0_word; };
    auto b_d = [=](data_size ds) { return ds == data_size::byte ? b : 0_word; };
    auto s_d = [=](mll_signedndesd sign) { return sign == mll_signedndesd::signed_ ? s : 0_word; };
    auto h_d = [=](data_size ds) { return ds == data_size::hword ? h : 0_word; };
    auto i2_d = [=](immediate_operand imop) { return imop == immediate_operand::on ? i2 : 0_word; };

    auto p_var = GENERATE(indexing::pre, indexing::post);
    auto u_var = GENERATE(direction::up, direction::down);
    auto w_var = GENERATE(write_back::on, write_back::off);
    
    SECTION("Type 1") {
        auto b_var = GENERATE(data_size::word, data_size::byte);
        
        SECTION("Shifts") {
            auto shift_var = GENERATE(shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx,
                                      shifts::lsl,  shifts::lsr,    shifts::asr,  shifts::ror);
            auto const expected = arm::instruction_spec::construct<ldr>(
                immediate_operand::off, shift_var, 
                u_var, p_var, w_var, 
                b_var, mll_signedndesd::unsigned_
            );
            auto const result = decode({
                skeleton1 | p_d(p_var) | u_d(u_var) | w_d(w_var) | shift_specifier(shift_var) | i1 | b_d(b_var)
            });
            CHECK(result.as_index() == expected.as_index());
        }
        SECTION("immediate_operand") {
            auto const expected = arm::instruction_spec::construct<ldr>(
                immediate_operand::on, shifts::null, 
                u_var, p_var, w_var, 
                b_var, mll_signedndesd::unsigned_
            );
            auto const result = decode({
                skeleton1 | p_d(p_var) | u_d(u_var) | w_d(w_var) | b_d(b_var)
            });
            CHECK(result.as_index() == expected.as_index());

        }

       
    }
    SECTION("Type 2") {
        auto i2_var = GENERATE(immediate_operand::on, immediate_operand::off);
        
        SECTION("Half word") {
            auto s_var = GENERATE(mll_signedndesd::signed_, mll_signedndesd::unsigned_);
            auto const expected = arm::instruction_spec::construct<ldr>(
                i2_var, shifts::null, u_var,
                p_var, w_var, data_size::hword, s_var
            );
            auto const result = decode({
                skeleton2 | p_d(p_var) | u_d(u_var) | w_d(w_var) | s_d(s_var) | h | i2_d(i2_var)
            });

            CHECK(expected.as_index() == result.as_index());
        }
        SECTION("Byte") {
            auto const expected = arm::instruction_spec::construct<ldr>(
                i2_var, shifts::null, u_var,
                p_var, w_var, data_size::byte, mll_signedndesd::signed_
            );
            auto const result = decode({
                skeleton2 | p_d(p_var) | u_d(u_var) | w_d(w_var) | s | i2_d(i2_var)
            });

            CHECK(expected.as_index() == result.as_index());
        }
    } 
}
TEST_CASE("Decoding arm str", "[cpu][decoding][arm]") {
    auto const skeleton1 = 0b1010'01'0'0'0'0'0'0'0000'0000'000000000000_word;
    auto const i1        = 0b0000'00'1'0'0'0'0'0'0000'0000'000000000000_word;
    auto const p         = 0b0000'00'0'1'0'0'0'0'0000'0000'000000000000_word;
    auto const u         = 0b0000'00'0'0'1'0'0'0'0000'0000'000000000000_word;
    auto const b         = 0b0000'00'0'0'0'1'0'0'0000'0000'000000000000_word;
    auto const w         = 0b0000'00'0'0'0'0'1'0'0000'0000'000000000000_word;
    auto const lsl       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0000'0000_word;
    auto const lsr       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0010'0000_word;
    auto const asr       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0100'0000_word;
    auto const ror       = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0110'0000_word;
    auto const reg_shift = 0b0000'00'0'0'0'0'0'0'0000'0000'0000'0001'0000_word;
    auto const shift_num = 0b0000'00'0'0'0'0'0'0'0000'0000'1111'0000'0000_word;

    auto const skeleton2 = 0b1010'000'0'0'0'0'0'0000'0000'0000'1'0'0'1'1100_word;
    auto const i2        = 0b0000'000'0'0'1'0'0'0000'0000'0000'0'0'0'0'0000_word;
    auto const h         = 0b0000'000'0'0'0'0'0'0000'0000'0000'0'0'1'0'0000_word;

    auto shift_specifier = [=](shifts shift) {
        auto result = word{0};
        switch(shift) {
            case shifts::null:  return lsl;
            case shifts::lsr32: return lsr;
            case shifts::asr32: return asr;
            case shifts::rrx:   return ror;
            case shifts::lsl:   return lsl | shift_num;
            case shifts::lsr:   return lsr | shift_num;
            case shifts::asr:   return asr | shift_num;
            case shifts::ror:   return ror | shift_num;
            case shifts::rslsl: return lsl | shift_num | reg_shift;
            case shifts::rslsr: return lsr | shift_num | reg_shift;
            case shifts::rsasr: return asr | shift_num | reg_shift;
            case shifts::rsror: return ror | shift_num | reg_shift;
            default: std::unreachable();
        }
    };
    auto p_d = [=](indexing ind) { return ind == indexing::pre ? p : 0_word; };
    auto u_d = [=](direction dir) { return dir == direction::up ? u : 0_word; };
    auto w_d = [=](write_back wb) { return wb == write_back::on ? w : 0_word; };
    auto b_d = [=](data_size ds) { return ds == data_size::byte ? b : 0_word; };
    auto h_d = [=](data_size ds) { return ds == data_size::hword ? h : 0_word; };
    auto i2_d = [=](immediate_operand imop) { return imop == immediate_operand::on ? i2 : 0_word; };

    auto p_var = GENERATE(indexing::pre, indexing::post);
    auto u_var = GENERATE(direction::up, direction::down);
    auto w_var = GENERATE(write_back::on, write_back::off);
    
    SECTION("Type 1") {
        auto b_var = GENERATE(data_size::word, data_size::byte);
        
        SECTION("Shifts") {
            auto shift_var = GENERATE(shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx,
                                      shifts::lsl,  shifts::lsr,    shifts::asr,  shifts::ror);
            auto const expected = arm::instruction_spec::construct<str>(
                immediate_operand::off, shift_var, 
                u_var, p_var, w_var, 
                b_var
            );
            auto const result = decode({
                skeleton1 | p_d(p_var) | u_d(u_var) | w_d(w_var) | shift_specifier(shift_var) | i1 | b_d(b_var)
            });
            CHECK(result.as_index() == expected.as_index());
        }
        SECTION("immediate_operand") {
            auto const expected = arm::instruction_spec::construct<str>(
                immediate_operand::on, shifts::null, 
                u_var, p_var, w_var, 
                b_var
            );
            auto const result = decode({
                skeleton1 | p_d(p_var) | u_d(u_var) | w_d(w_var) | b_d(b_var)
            });
            CHECK(result.as_index() == expected.as_index());

        }

       
    }
    SECTION("Type 2") {
        auto i2_var = GENERATE(immediate_operand::on, immediate_operand::off);
        
        SECTION("Half word") {
            auto s_var = GENERATE(mll_signedndesd::signed_, mll_signedndesd::unsigned_);
            auto const expected = arm::instruction_spec::construct<str>(
                i2_var, shifts::null, u_var,
                p_var, w_var, data_size::hword
            );
            auto const result = decode({
                skeleton2 | p_d(p_var) | u_d(u_var) | w_d(w_var) | h | i2_d(i2_var)
            });

            CHECK(expected.as_index() == result.as_index());
        }
    } 
}

TEST_CASE("Decoding arm stm", "[arm][cpu][decoding]") {
    auto const skeleton = 0b0010'100'0'0'0'0'0'1111'0001101001001110_word;
    auto const p        = 0b0000'000'1'0'0'0'0'0000'0000000000000000_word;
    auto const u        = 0b0000'000'0'1'0'0'0'0000'0000000000000000_word;
    auto const s        = 0b0000'000'0'0'1'0'0'0000'0000000000000000_word;
    auto const w        = 0b0000'000'0'0'0'1'0'0000'0000000000000000_word;

    auto p_d = [=](indexing ind) { return ind == indexing::pre ? p : 0_word; };
    auto u_d = [=](direction dir) { return dir == direction::up ? u : 0_word; };
    auto w_d = [=](write_back wb) { return wb == write_back::on ? w : 0_word; };
    auto s_d = [=](s_bit sb) { return sb == s_bit::on ? s : 0_word; };

    auto p_var = GENERATE(indexing::pre, indexing::post);
    auto u_var = GENERATE(direction::down, direction::up);
    auto w_var = GENERATE(write_back::on, write_back::off);
    auto s_var = GENERATE(s_bit::on, s_bit::off);

    auto const expected = arm::instruction_spec::construct<stm>(
        s_var, u_var, w_var, p_var
    );
    auto const result   = decode({skeleton | p_d(p_var) | u_d(u_var) | w_d(w_var) | s_d(s_var)});

    CHECK(expected.as_index() == result.as_index());
}

TEST_CASE("Decoding arm ldm", "[arm][cpu][decoding]") {
    auto const skeleton = 0b0010'100'0'0'0'0'1'1111'0001101001001110_word;
    auto const p        = 0b0000'000'1'0'0'0'0'0000'0000000000000000_word;
    auto const u        = 0b0000'000'0'1'0'0'0'0000'0000000000000000_word;
    auto const s        = 0b0000'000'0'0'1'0'0'0000'0000000000000000_word;
    auto const w        = 0b0000'000'0'0'0'1'0'0000'0000000000000000_word;

    auto p_d = [=](indexing ind) { return ind == indexing::pre ? p : 0_word; };
    auto u_d = [=](direction dir) { return dir == direction::up ? u : 0_word; };
    auto w_d = [=](write_back wb) { return wb == write_back::on ? w : 0_word; };
    auto s_d = [=](s_bit sb) { return sb == s_bit::on ? s : 0_word; };

    auto p_var = GENERATE(indexing::pre, indexing::post);
    auto u_var = GENERATE(direction::down, direction::up);
    auto w_var = GENERATE(write_back::on, write_back::off);
    auto s_var = GENERATE(s_bit::on, s_bit::off);

    auto const expected = arm::instruction_spec::construct<ldm>(
        s_var, u_var, w_var, p_var
    );
    auto const result   = decode({skeleton | p_d(p_var) | u_d(u_var) | w_d(w_var) | s_d(s_var)});

    CHECK(expected.as_index() == result.as_index());
}

TEST_CASE("Decoding arm swi", "[arm][cpu][decoding]") {
    auto const expected = arm::instruction_spec::construct<swi>();
    auto const result   = decode({0b0110'1111'11001101'01011001'11101100_word});

    CHECK(expected.as_index() == result.as_index());
}

TEST_CASE("Decoding arm swp", "[arm][cpu][decoding]") {
    auto const skeleton = 0b0110'00010'0'00'1111'1111'0000'1001'0000_word;
    auto const b        = 0b0000'00000'1'00'0000'0000'0000'0000'0000_word;

    auto b_d = [=](data_size ds) { return ds == data_size::byte ? b : 0_word; };

    auto b_var = GENERATE(data_size::word, data_size::byte);

    auto const expected = arm::instruction_spec::construct<swp>(b_var);
    auto const result   = decode({skeleton | b_d(b_var)});

    CHECK(expected.as_index() == result.as_index());
}

