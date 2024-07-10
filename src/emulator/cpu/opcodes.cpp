#include "emulator/cpu/opcodes.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include "emulator/cpudefines.hpp"
#include "fmt/base.h"
#include <algorithm>
#include <array>
#include <ranges>
#include <tuple>
#include <utility>

namespace fgba {

auto set_shift(u32 base, std::tuple<cpu::shifts> opts, cpu::shifts shift) {
    base |= 1;
    std::get<cpu::shifts>(opts) = shift;
    return std::tuple{base, opts};
}
namespace composing {
template<typename F, typename T>
struct populator {
    constexpr populator(F&& f, std::vector<T>&& vals) 
        : m_populatable{std::move(f)}, m_values{std::move(vals)} {}
    constexpr auto operator()(auto tup_args) const {
        std::apply([&](auto... args) {
            for (auto value : m_values) {
                m_populatable(args..., value);
            }
        }, tup_args);
    }
    F m_populatable;
    std::vector<T> m_values;
};
template<typename F>
struct finalizer {
    constexpr finalizer(F&& f) : func{std::forward<F>(f)} {}
    constexpr auto operator()(auto tup_args) const {
        std::apply(func, tup_args);
    }
    F func;
};

template<typename F1, typename F2>
constexpr auto operator|(F1&& f1, F2&& f2) {
    // return composer{f1, f2};
    return [f1_ = std::forward<F1>(f1), f2_ = std::forward<F2>(f2)](auto&&... args) { // NOLINT 
        return f1_(f2_(std::forward<decltype(args)>(args)...));
    }; 
}
template<typename F, typename T>
constexpr auto operator|(F&& f, std::vector<T>&& vals) {
    return populator{std::move(f), std::move(vals)}; // NOLINT
}
} // namespace composing
namespace cpu::arm {

// template<instruction_spec::set Instr>
// consteval auto test_extraction_and_uniqueness(auto... switches) 
//     -> bool {
//     return std::tuple{switches...} == instruction_spec::construct<Instr>(switches...).template switches<Instr>();
// }
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::b>());
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::bl>());
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::bx>());
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::off,
//     shifts::null,
//     s_bit::off
// ));
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::on,
//     shifts::null,
//     s_bit::off
// ));
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::off,
//     shifts::null,
//     s_bit::on
// ));
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::off,
//     shifts::null,
//     s_bit::off>
// ));
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::off,
//     shifts::rsasr,
//     s_bit::off
// ));
// static_assert(test_extraction_and_uniqueness<instruction_spec::set::and_>(
//     immediate_operand::off,
//     shifts::rsasr,
//     s_bit::on
// ));
}

namespace cpu::arm {

namespace decoding {
using namespace composing;
using enum instruction_spec::set;
struct instruction_info {
    word mask;
    word opcode;
};
using lut = std::array<instruction_info, instruction_spec::count()>;


//dummy namespace just to qualify functions
namespace qual {

consteval auto generate_opcode_lut()
    -> lut;


consteval auto asign_branch_opcodes(lut& lut)
    -> void;


consteval auto asign_data_processing(lut& lut)
    -> void;
template<instruction_spec::set>
consteval auto create_and_like(lut&, instruction_info, word id)
    -> void;
template<instruction_spec::set>
consteval auto create_tst_like(lut&, instruction_info, word id)
    -> void;
template<instruction_spec::set>
consteval auto create_s_bit_variation(lut&, instruction_info, s_bit)
    -> void;
template<instruction_spec::set>
consteval auto create_special_shift_variation(lut&, instruction_info, s_bit, cpu::shifts, word id)
    -> void;
template<instruction_spec::set>
consteval auto create_register_shift_variation(lut&, instruction_info, s_bit, cpu::shifts, word id)
    -> void;
template<instruction_spec::set>
consteval auto create_normal_shift_variation(lut&, instruction_info, s_bit, cpu::shifts, word id)
    -> void;


consteval auto asign_psr_transfer(lut&)
    -> void;
consteval auto create_mrs(lut&)
    -> void;
consteval auto create_msr(lut&)
    -> void;
consteval auto create_msr_full(lut&)
    -> void;
consteval auto create_msr_flag_bits(lut&)
    -> void;


consteval auto asign_multiplication(lut& lut)
    -> void;
consteval auto create_mul(lut&)
    -> void;
consteval auto create_mll(lut&)
    -> void;
} // namespace qual

consteval auto qual::asign_branch_opcodes(lut& lut)
    -> void {
    lut[instruction_spec::construct<b>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'1010'0000'0000'0000'0000'0000'0000_word,
    };
    lut[instruction_spec::construct<bl>().as_index()] = {
        .mask   = 0b0000'1111'0000'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'1011'0000'0000'0000'0000'0000'0000_word,
    };
    lut[instruction_spec::construct<bx>().as_index()] = {
        .mask   = 0b0000'1111'1111'1111'1111'1111'1111'0000_word,
        .opcode = 0b0000'0001'0010'1111'1111'1111'0001'0000_word,
    };
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_special_shift_variation(lut& lut, instruction_info base, s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[11, 4]    = 0b1111'1111_word;
    base.opcode[6, 5]   = id;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_register_shift_variation(lut& lut, instruction_info base, s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[7, 4]   = 0b1111_word;
    base.opcode[6, 5] = id;
    base.opcode[4]    = 1_bit;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_normal_shift_variation(lut& lut, instruction_info base, s_bit s, cpu::shifts shift, word const id)
    -> void {
    base.mask[6, 4]   = 0b111_word;
    base.opcode[6, 5] = id;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift, s).as_index()] = base;
    } else {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::off, shift).as_index()] = base;
    }
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_s_bit_variation(lut& lut, instruction_info base, s_bit s)
    -> void {
    base.opcode[20] = s == s_bit::on ? 1_bit : 0_bit;
    qual::create_special_shift_variation <InstructionBase>(lut, base, s, shifts::null,  0b00_word);
    qual::create_special_shift_variation <InstructionBase>(lut, base, s, shifts::lsr32, 0b01_word);
    qual::create_special_shift_variation <InstructionBase>(lut, base, s, shifts::asr32, 0b10_word);
    qual::create_special_shift_variation <InstructionBase>(lut, base, s, shifts::rrx,   0b11_word);
    qual::create_register_shift_variation<InstructionBase>(lut, base, s, shifts::rslsl, 0b00_word);
    qual::create_register_shift_variation<InstructionBase>(lut, base, s, shifts::rslsr, 0b01_word);
    qual::create_register_shift_variation<InstructionBase>(lut, base, s, shifts::rsasr, 0b10_word);
    qual::create_register_shift_variation<InstructionBase>(lut, base, s, shifts::rsror, 0b11_word);
    qual::create_normal_shift_variation  <InstructionBase>(lut, base, s, shifts::lsl,   0b00_word);
    qual::create_normal_shift_variation  <InstructionBase>(lut, base, s, shifts::lsr,   0b01_word);
    qual::create_normal_shift_variation  <InstructionBase>(lut, base, s, shifts::asr,   0b10_word);
    qual::create_normal_shift_variation  <InstructionBase>(lut, base, s, shifts::ror,   0b11_word);
    base.opcode[25] = 1_bit;
    if constexpr (std::to_underlying(InstructionBase) < std::to_underlying(tst)) {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null, s).as_index()] = base;
    } else {
        lut[instruction_spec::construct<InstructionBase>(cpu::immediate_operand::on, cpu::shifts::null).as_index()] = base;
    }
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_and_like(lut& lut, instruction_info base, word const id)
    -> void {
    base.opcode[24, 21] = id;
    create_s_bit_variation<InstructionBase>(lut, base, s_bit::on);
    create_s_bit_variation<InstructionBase>(lut, base, s_bit::off);
}
template<instruction_spec::set InstructionBase>
consteval auto qual::create_tst_like(lut& lut, instruction_info base, word const id)
    -> void {
    base.opcode[24, 21] = id;
    create_s_bit_variation<InstructionBase>(lut, base, s_bit::on);
}

consteval auto qual::asign_data_processing(lut &lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'1111'1111'0000'0000'0000'0000'0000_word,
        .opcode = 0b0000'0000'0000'0000'0000'0000'0000'0000_word,
    };
    qual::create_and_like<and_>(lut, base, 0b0000_word);
    qual::create_and_like<eor> (lut, base, 0b0001_word);
    qual::create_and_like<sub> (lut, base, 0b0010_word);
    qual::create_and_like<rsb> (lut, base, 0b0011_word);
    qual::create_and_like<add> (lut, base, 0b0100_word);
    qual::create_and_like<adc> (lut, base, 0b0101_word);
    qual::create_and_like<sbc> (lut, base, 0b0110_word);
    qual::create_and_like<rsc> (lut, base, 0b0111_word);
    qual::create_tst_like<tst> (lut, base, 0b1000_word);
    qual::create_tst_like<teq> (lut, base, 0b1001_word);
    qual::create_tst_like<cmp> (lut, base, 0b1010_word);
    qual::create_tst_like<cmn> (lut, base, 0b1011_word);
    qual::create_and_like<orr> (lut, base, 0b1100_word);
    qual::create_and_like<mov> (lut, base, 0b1101_word);
    qual::create_and_like<bic> (lut, base, 0b1110_word);
    qual::create_and_like<mvn> (lut, base, 0b1111_word);
}
template<unsigned Bit, typename FlagT, FlagT TrueFlagV>
inline constexpr auto flag_bit = [](instruction_info base, auto opts, FlagT flag) {
    base.opcode[Bit] = flag == TrueFlagV;
    std::get<FlagT>(opts) = flag;
    return std::tuple{base, opts};
};

template<instruction_spec::set Instr>
consteval auto lazy_asign_opcode_unsafe(lut& lut) {
    using opts = instruction_spec::flag_bundle_t<Instr>;
    return [lut_ = &lut](instruction_info result, opts opts) { // NOLINT
        (*lut_)[std::apply([](auto... args){ return instruction_spec::construct<Instr>(args...); }, opts).as_index()] = result;
    };
}
consteval auto qual::create_mrs(lut& lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'11111'1'111111'0000'1111'1111'1111_word,
        .opcode = 0b0000'00010'0'001111'0000'0000'0000'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<mrs>;
    (finalizer{lazy_asign_opcode_unsafe<mrs>(lut)}
        |flag_bit<22, which_psr, which_psr::spsr>
            |std::vector{which_psr::cpsr, which_psr::spsr}
    )(std::tuple{base, opts{}});
}


consteval auto qual::create_msr(lut& lut)
    -> void {
    // qual::create_msr_full(lut);
    qual::create_msr_flag_bits(lut);
}

consteval auto qual::create_msr_full(lut& lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'11111'1'1111111111'11111111'0000_word,
        .opcode = 0b0000'00010'0'1010011111'00000000'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<msr>;
    (finalizer{lazy_asign_opcode_unsafe<msr>(lut)}
        |flag_bit<22, which_psr, which_psr::spsr>
            |std::vector{which_psr::cpsr, which_psr::spsr}
    )(std::tuple{base, opts{}});
}
template<typename Opts>
consteval auto set_i_bit_msr(instruction_info base, Opts opts, immediate_operand im_op) {
    base.opcode[25] = im_op == immediate_operand::on;
    std::get<immediate_operand>(opts) = im_op;
    if (im_op == immediate_operand::off) {
        base.mask[11, 4] = 0b1111'1111_word;
    }
    return std::tuple{base, opts};
}
consteval auto qual::create_msr_flag_bits(lut& lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'11'1'11'1'1100000000'0000'0000'0000_word,
        .opcode = 0b0000'00'0'10'0'1000000000'0000'0000'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<msr>;
    auto options = opts{};
    (finalizer{lazy_asign_opcode_unsafe<msr>(lut)}
        |flag_bit<22, which_psr, which_psr::spsr>
            |std::vector{which_psr::spsr, which_psr::cpsr}
        |set_i_bit_msr<opts>
            |std::vector{immediate_operand::on, immediate_operand::off}
    )(std::tuple{base, options});
}

consteval auto qual::asign_psr_transfer(lut& lut) 
    -> void {
    qual::create_mrs(lut);
    qual::create_msr(lut);
}
consteval auto qual::create_mul(lut& lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'111111'1'1'0000'0000'0000'1111'0000_word,
        .opcode = 0b0000'000000'0'0'0000'0000'0000'1001'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<mul>;
    (finalizer{lazy_asign_opcode_unsafe<mul>(lut)}
        |flag_bit<20, s_bit, s_bit::on>
            |std::vector{s_bit::on, s_bit::off}
        |flag_bit<21, accumulate, accumulate::on>
            |std::vector{accumulate::on, accumulate::off}
    )(std::tuple{base, opts{}});
}

consteval auto qual::create_mll(lut& lut)
    -> void {
    auto base = instruction_info{
        .mask   = 0b0000'11111'1'1'1'0000'0000'0000'1111'0000_word,
        .opcode = 0b0000'00001'0'0'0'0000'0000'0000'1001'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<mll>;
    (finalizer{lazy_asign_opcode_unsafe<mll>(lut)}
        |flag_bit<20, s_bit, s_bit::on>
            |std::vector{s_bit::on, s_bit::off}
        |flag_bit<21, accumulate, accumulate::on>
            |std::vector{accumulate::on, accumulate::off}
        |flag_bit<22, mll_signedndesd, mll_signedndesd::signed_>
            |std::vector{mll_signedndesd::signed_, mll_signedndesd::unsigned_}
    )(std::tuple{base, opts{}});
}
consteval auto qual::asign_multiplication(lut& lut)
    -> void {
    qual::create_mul(lut);
    qual::create_mll(lut);
}

inline constexpr auto shift_switch = [](instruction_info base, auto opts, shifts shift) {
    switch (shift) {
    using enum shifts;
        case null: {
            base.mask[11, 4]   = ~0_word;
            base.opcode[11, 4] = 0_word;
            break;
        }
        case lsr32: {
            base.mask[11, 4]   = ~0_word;
            base.opcode[11, 7] = 0_word;
            base.opcode[6, 5]  = 0b01_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case asr32: {
            base.mask[11, 4]   = ~0_word;
            base.opcode[11, 7] = 0_word;
            base.opcode[6, 5]  = 0b10_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case rrx: {
            base.mask[11, 4]   = ~0_word;
            base.opcode[11, 7] = 0_word;
            base.opcode[6, 5]  = 0b11_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case lsl: {
            base.mask[6, 4]    = ~0_word;
            base.opcode[6, 4]  = 0_word;
            break;
        }
        case lsr: {
            base.mask[6, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b01_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case asr: {
            base.mask[6, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b10_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case ror: {
            base.mask[6, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b11_word;
            base.opcode[4]     = 0_bit;
            break;
        }
        case rslsl: {
            base.mask[7, 4]    = ~0_word;
            base.opcode[7, 4]  = 1_word;
            break;
        }
        case rslsr: {
            base.mask[7, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b01_word;
            base.opcode[4]     = 1_bit;
            base.opcode[7]     = 0_bit;
            break;
        }
        case rsasr: {
            base.mask[7, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b10_word;
            base.opcode[4]     = 1_bit;
            base.opcode[7]     = 0_bit;
            break;
        }
        case rsror: {
            base.mask[7, 4]    = ~0_word;
            base.opcode[6, 5]  = 0b11_word;
            base.opcode[4]     = 1_bit;
            base.opcode[7]     = 0_bit;
            break;
        }
    }
    std::get<shifts>(opts) = shift;
    return std::tuple{base, opts};
};


consteval auto create_ldr_type1(lut& lut) {
    auto base = instruction_info{
        .mask   = 0b0000'11'1'1'1'1'1'1'0000'0000'000000000000_word,
        .opcode = 0b0000'01'0'0'0'0'0'1'0000'0000'000000000000_word,
    };
    using opts = instruction_spec::flag_bundle_t<ldr>;
    auto options = opts{};
    std::get<mll_signedndesd>(options) = mll_signedndesd::unsigned_;
    auto common_composition = finalizer{lazy_asign_opcode_unsafe<ldr>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::pre, indexing::post}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
        |flag_bit<22, data_size, data_size::byte>
            |std::vector{data_size::byte, data_size::word};
    (common_composition
        |flag_bit<25, immediate_operand, immediate_operand::off>
            |std::vector{immediate_operand::off}
        |shift_switch 
            |std::vector{shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx,
                         shifts::lsl,  shifts::lsr,   shifts::asr,   shifts::ror}
    )(std::tuple{base, options});
    (common_composition
        |flag_bit<25, immediate_operand, immediate_operand::off>
            |std::vector{immediate_operand::on}
    )(std::tuple{base, options});
}
consteval auto create_ldr_type2(lut& lut) {
    auto base = instruction_info{
        .mask   = {word::create_mask(27, 20) | word::create_mask(7, 4)},
        .opcode = 0b0000'000'0'0'0'0'1'0000'0000'0000'1'0'0'1'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<ldr>;
    auto common = finalizer{lazy_asign_opcode_unsafe<ldr>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::pre, indexing::post}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
        |[](instruction_info base, auto opts, immediate_operand im_op) {
            std::get<immediate_operand>(opts) = im_op;
            if (not (base.opcode[22] = im_op != immediate_operand::on)) { //NOLINT
                base.mask[11, 8] = ~0_word;
            }
            return std::tuple{base, opts};
        }
            |std::vector{immediate_operand::on, immediate_operand::off};
    (common
        |flag_bit<5, data_size, data_size::hword>
            |std::vector{data_size::hword}
        |flag_bit<6, mll_signedndesd, mll_signedndesd::signed_>
            |std::vector{mll_signedndesd::signed_, mll_signedndesd::unsigned_}
    )(std::tuple{base, opts{}});
    (common
        |flag_bit<5, data_size, data_size::hword>
            |std::vector{data_size::byte}
        |flag_bit<6, mll_signedndesd, mll_signedndesd::signed_>
            |std::vector{mll_signedndesd::signed_}
    )(std::tuple{base, opts{}});
;
}

consteval auto asign_load(lut& lut) {
    create_ldr_type1(lut);
    create_ldr_type2(lut);
}
consteval auto create_str_type1(lut& lut) {
    auto base = instruction_info{
        .mask   = 0b0000'11'1'1'1'1'1'1'0000'0000'000000000000_word,
        .opcode = 0b0000'01'0'0'0'0'0'0'0000'0000'000000000000_word,
    };
    using opts = instruction_spec::flag_bundle_t<str>;
    auto options = opts{};
    auto common_composition = finalizer{lazy_asign_opcode_unsafe<str>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::pre, indexing::post}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
        |flag_bit<22, data_size, data_size::byte>
            |std::vector{data_size::byte, data_size::word};
    (common_composition
        |flag_bit<25, immediate_operand, immediate_operand::off>
            |std::vector{immediate_operand::off}
        |shift_switch 
            |std::vector{shifts::null, shifts::lsr32, shifts::asr32, shifts::rrx,
                         shifts::lsl,  shifts::lsr,   shifts::asr,   shifts::ror}
    )(std::tuple{base, options});
    (common_composition
        |flag_bit<25, immediate_operand, immediate_operand::off>
            |std::vector{immediate_operand::on}
    )(std::tuple{base, options});
}
consteval auto create_str_type2(lut& lut) {
    auto base = instruction_info{
        .mask   = {word::create_mask(27, 20) | word::create_mask(7, 4)},
        .opcode = 0b0000'000'0'0'0'0'0'0000'0000'0000'1'0'0'1'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<str>;
    auto common = finalizer{lazy_asign_opcode_unsafe<str>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::pre, indexing::post}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
        |[](instruction_info base, auto opts, immediate_operand im_op) {
            std::get<immediate_operand>(opts) = im_op;
            if (not (base.opcode[22] = im_op != immediate_operand::on)) { //NOLINT
                base.mask[11, 8] = ~0_word;
            }
            return std::tuple{base, opts};
        }
            |std::vector{immediate_operand::on, immediate_operand::off};
    (common
        |flag_bit<5, data_size, data_size::hword>
            |std::vector{data_size::hword}
    )(std::tuple{base, opts{}});
}
consteval auto asign_str(lut& lut) {
    create_str_type1(lut);
    create_str_type2(lut);
}

consteval auto asign_stm(lut& lut) {
    auto base = instruction_info{
        .mask   = 0b0000'111'1'1'1'1'1'0000'0000000000000000_word,
        .opcode = 0b0000'100'0'0'0'0'0'0000'0000000000000000_word,
    };
    using opts = instruction_spec::flag_bundle_t<stm>;
    (finalizer{lazy_asign_opcode_unsafe<stm>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::post, indexing::pre}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<22, s_bit, s_bit::on>
            |std::vector{s_bit::on, s_bit::off}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
    )(std::tuple{base, opts{}});
}
consteval auto asign_ldm(lut& lut) {
    auto base = instruction_info{
        .mask   = 0b0000'111'1'1'1'1'1'0000'0000000000000000_word,
        .opcode = 0b0000'100'0'0'0'0'1'0000'0000000000000000_word,
    };
    using opts = instruction_spec::flag_bundle_t<ldm>;
    (finalizer{lazy_asign_opcode_unsafe<ldm>(lut)}
        |flag_bit<24, indexing, indexing::pre>
            |std::vector{indexing::post, indexing::pre}
        |flag_bit<23, direction, direction::up>
            |std::vector{direction::up, direction::down}
        |flag_bit<22, s_bit, s_bit::on>
            |std::vector{s_bit::on, s_bit::off}
        |flag_bit<21, write_back, write_back::on>
            |std::vector{write_back::on, write_back::off}
    )(std::tuple{base, opts{}});
}

consteval auto asign_swp(lut& lut) {
    auto const base = instruction_info{
        .mask   = 0b0000'11111'1'11'0000'0000'1111'1111'0000_word,
        .opcode = 0b0000'00010'0'00'0000'0000'0000'1001'0000_word,
    };
    using opts = instruction_spec::flag_bundle_t<swp>;
    (finalizer{lazy_asign_opcode_unsafe<swp>(lut)}
        |flag_bit<22, data_size, data_size::byte>
            |std::vector{data_size::byte, data_size::word}
    )(std::tuple{base, opts{}});
}

consteval auto asign_swi(lut& lut) {
    auto const base = instruction_info{
        .mask   = 0b0000'1111'00000000'00000000'00000000_word,
        .opcode = 0b0000'1111'00000000'00000000'00000000_word,
    };
    lut[instruction_spec::construct<swi>().as_index()] = base;
}

consteval auto qual::generate_opcode_lut()
    -> lut {
    lut ret{};
    
    qual::asign_branch_opcodes(ret);
    qual::asign_data_processing(ret);
    qual::asign_psr_transfer(ret);
    qual::asign_multiplication(ret);
    asign_load(ret);
    asign_str(ret);
    asign_stm(ret);
    asign_ldm(ret);
    asign_swp(ret);
    asign_swi(ret);

    return ret;

}

constexpr auto instruction_info_lut = qual::generate_opcode_lut();
}
namespace {
constexpr auto masked_decode(arm::instruction const instruction) noexcept -> instruction_spec {
    std::size_t index = 0;
    for(; index < decoding::instruction_info_lut.size(); ++index) {
        auto const [mask, opcode] = decoding::instruction_info_lut[index];
        auto const new_mask = mask.mask_in(27, 20) | mask.mask_in(11, 4);
        auto const new_opcode = opcode.mask_in(27, 20) | opcode.mask_in(11, 4);
        if ((instruction & new_mask) == new_opcode) break;
    }
    return instruction_spec{index};
}
}

    auto to_index = [](instruction const instruction) {
        auto result = word{0};
        result[3, 0]  = instruction[7, 4];
        result[4]     = instruction[11, 8] != 0_word;
        result[12, 5] = instruction[27, 20];
        return result.as<std::size_t>();
    };
auto generate_offset_lut() {
    std::array<std::optional<u16>, 0b1111'1111'1111'1> checker{};
    

    auto significant_bits = instruction{instruction::create_mask(27, 20) | instruction::create_mask(11, 4)};
    auto runner = instruction{0};
    do {
        auto index = to_index(runner);
        auto decode_result_index = static_cast<u16>(masked_decode(runner).as_index());
        if (not checker[index].has_value()) {
            checker[index] = decode_result_index;
        } else if (*checker[index] != decode_result_index) {
            fmt::println("Uh-oh, {:#034b}", runner.value);
        }

        
        runner = {(runner - significant_bits) & significant_bits};
    } while(runner);
    auto ret = std::array<u16, 0b1111'1111'1111'1>{};
    for (std::size_t i = 0; i < checker.size(); ++i) {
        ret[i] = checker[i].value_or(69);
    }
    return ret;
}


// static auto const offset_lut = generate_offset_lut();
} // namespace 


 // namespace cpu::arm
namespace cpu {
auto decode(arm::instruction const instruction) noexcept -> arm::instruction_spec {
    // return {arm::offset_lut[arm::to_index(instruction)]};
    return arm::masked_decode(instruction);
}
auto mask_for(arm::instruction_spec spec) noexcept -> word {
    return arm::decoding::instruction_info_lut[spec.as_index()].mask;
}
auto opcode_for(arm::instruction_spec spec) noexcept -> word {
    return arm::decoding::instruction_info_lut[spec.as_index()].opcode;
}
}
} // namespace fgba
