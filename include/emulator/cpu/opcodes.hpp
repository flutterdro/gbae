#ifndef OP_CODES_HPP_
#define OP_CODES_HPP_

#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
#include "utility/fatexception.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include <cstddef>
#include <utility>

namespace fgba::cpu {

class arm_instruction {
public:
    enum class set : u32;
    constexpr arm_instruction(set);
    constexpr arm_instruction(set, immediate_operand, shifts, s_bit);
    [[nodiscard]]constexpr auto as_index() const noexcept -> size_t;
    [[nodiscard]]constexpr auto get_handle() const noexcept -> set;
    [[nodiscard]]constexpr auto get_base() const noexcept -> arm_instruction;
    [[nodiscard]]static consteval auto count() noexcept -> size_t;
private:
  set m_instruction;
};

class thumb_instruction {
public:
    enum class set : u32;
    constexpr thumb_instruction(set);
    [[nodiscard]]constexpr auto as_index() const noexcept -> size_t;
    [[nodiscard]]constexpr auto get_handle() const noexcept -> set;
    [[nodiscard]]constexpr auto get_base() const noexcept -> thumb_instruction;
    [[nodiscard]]static consteval auto count() noexcept -> size_t;
private:
  set m_instruction;
};

[[nodiscard]]auto mask_for(arm_instruction) noexcept -> u32;
[[nodiscard]]auto mask_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto opcode_for(arm_instruction) noexcept -> u32;
[[nodiscard]]auto opcode_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto decode(u32) noexcept -> arm_instruction;
[[nodiscard]]auto decode(u16) noexcept -> thumb_instruction;

namespace detail {
template<typename... Enums>
inline constexpr u32 offset = ((std::to_underlying(Enums::count)) * ...);
} // namespace detail

enum class arm_instruction::set : u32 {
    b = 0, bx, bl,
    and_, //NOLINT
    orr = and_ + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    eor = orr  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    bic = eor  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    add = bic  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    adc = add  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    sub = adc  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    sbc = sub  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    rsb = sbc  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    rsc = rsb  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    mov = rsc  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    mvn = mov  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    tst = mvn  + detail::offset<s_bit, shifts> + detail::offset<s_bit, immediate_operand>,
    teq = tst  + detail::offset<shifts>        + detail::offset<immediate_operand>,
    cmp = teq  + detail::offset<shifts>        + detail::offset<immediate_operand>,
    cmn = cmp  + detail::offset<shifts>        + detail::offset<immediate_operand>,
    mul = cmn  + detail::offset<shifts>        + detail::offset<immediate_operand>,
    undefined,
    
    count,
};
constexpr arm_instruction::arm_instruction(set instr) 
    : m_instruction{instr} {}
constexpr arm_instruction::arm_instruction(set base, immediate_operand i, shifts shift, s_bit s) 
    : m_instruction{0} {
    using enum set;
    auto tu = [](set i) { return std::to_underlying(i); };
    if (tu(base) < tu(and_) or tu(base) >= tu(mul)) { throw fgba::runtime_error("Invalid constructor used"); }
    if (tu(base) >= tu(tst)) s = s_bit::off;
    m_instruction = static_cast<set>(
        std::to_underlying(base) +
        std::to_underlying(s) * (detail::offset<shifts> + detail::offset<immediate_operand>) +
        (i == immediate_operand::on ? detail::offset<shifts> : std::to_underlying(shift))
    );
}

constexpr auto arm_instruction::as_index() const noexcept -> size_t { return std::to_underlying(m_instruction); }
consteval auto arm_instruction::count() noexcept -> size_t { return std::to_underlying(set::count); }
constexpr auto arm_instruction::get_base() const noexcept -> arm_instruction {
    using enum set;
    auto tu = [](set i) { return std::to_underlying(i); };
    auto adjust = [tu](set start, set instr, size_t offset) {
        return static_cast<set>(
            (tu(instr) - tu(start)) / offset * offset + tu(start)
        );
    };
    if (tu(m_instruction) < tu(and_)) return *this;
    if (tu(m_instruction) < tu(tst)) return adjust(and_, m_instruction, tu(orr) - tu(and_));
    if (tu(m_instruction) < tu(mul)) return adjust(tst, m_instruction, tu(teq) - tu(tst));
    return undefined;
}

} // namespace fgba::cpu

namespace fgba::cpu {
//struct arm_instruction {
//  u32 mask;
//  u32 opcode;
//};
#define GEN_SPECIFIC_SHIFT(instruction_base, shift_name, shift_mask_exclude,   \
                           shift_code)                                         \
  do {                                                                         \
    ret[instruction_base##shift_name] = {                                      \
        .mask = ret[instruction_base].mask ^ ((shift_mask_exclude) << 4),        \
        .opcode = ret[instruction_base].opcode | ((shift_code) << 4),            \
    };                                                                         \
  } while (false)
#define GEN_SHIFT_VARIATIONS(instruction_base)                                 \
  do {                                                                         \
    GEN_SPECIFIC_SHIFT(instruction_base, lsr32, 0b00000'00'0, 0b0000'0'01'0);  \
    GEN_SPECIFIC_SHIFT(instruction_base, asr32, 0b00000'00'0, 0b0000'0'10'0);  \
    GEN_SPECIFIC_SHIFT(instruction_base, rrx, 0b00000'00'0, 0b0000'0'11'0);    \
    GEN_SPECIFIC_SHIFT(instruction_base, lsl, 0b11111'00'0, 0b0000'0'00'0);    \
    GEN_SPECIFIC_SHIFT(instruction_base, lsr, 0b11111'00'0, 0b0000'0'01'0);    \
    GEN_SPECIFIC_SHIFT(instruction_base, asr, 0b11111'00'0, 0b0000'0'10'0);    \
    GEN_SPECIFIC_SHIFT(instruction_base, ror, 0b11111'00'0, 0b0000'0'11'0);    \
    GEN_SPECIFIC_SHIFT(instruction_base, rslsl, 0b11110'00'0, 0b0000'1'00'1);  \
    GEN_SPECIFIC_SHIFT(instruction_base, rslsr, 0b11110'00'0, 0b0000'1'01'1);  \
    GEN_SPECIFIC_SHIFT(instruction_base, rsasr, 0b11110'00'0, 0b0000'1'10'1);  \
    GEN_SPECIFIC_SHIFT(instruction_base, rsror, 0b11110'00'0, 0b0000'1'11'1);  \
  } while (false)
constexpr auto arm_instruction_init() {
  std::array<arm_instruction, arm_instruction_set::undefined> ret;
//  ret[arm_instruction_set::bx] = {
//      .mask = 0x0f'ff'ff'f0,
//      .opcode = 0x01'2f'ff'10,
//  };
//  ret[arm_instruction_set::b] = {
//      .mask = 0x0f'00'00'00,
//      .opcode = 0x0a'00'00'00,
//  };
//  ret[arm_instruction_set::bl] = {
//      .mask = 0x0f'00'00'00,
//      .opcode = 0x0b'00'00'00,
//  };
//  ret[arm_instruction_set::tst] = {
//      .mask = 0x0f'ff'0f'f0,
//      .opcode = 0x0f'11'00'00,
//  };
//  //GEN_SHIFT_VARIATIONS(arm_instruction_set::tst);
//  ret[arm_instruction_set::tsti] = {
//      .mask = 0x0f'ff'00'00,
//      .opcode = 0x0f'11'00'00,
//  };
//  ret[arm_instruction_set::mov] = {
//      .mask = 0x0,
//      .opcode = 0x0,
//  };
// // GEN_SHIFT_VARIATIONS(mov);
//  // ret[arm_instruction_set::movs] = {
//  //     .mask   = 0x0,
  //     .opcode = 0x0,
  // };
  // GEN_SHIFT_VARIATIONS(movs);
  // ret[arm_instruction_set::mvns] = {
  //     .mask   = 0x0,
  //     .opcode = 0x0,
  // };
  // GEN_SHIFT_VARIATIONS(mvns);

  return ret;
};
#undef GEN_SHIFT_VARIATIONS
#undef GEN_SPECIFIC_SHIFT

const std::array opcodes = arm_instruction_init();

} // namespace fgba::cpu

#endif
