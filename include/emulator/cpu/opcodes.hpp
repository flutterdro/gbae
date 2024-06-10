#ifndef OP_CODES_HPP_
#define OP_CODES_HPP_

#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
#include "utility/fatexception.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include <cstddef>
#include <type_traits>
#include <utility>


// This code is a mess which I hope would be nice to use 
//
// So it is basically a glue which is used to connect execution and decoding
// since each instruction has a bunch of bells and whistles and i treat them as seperate instructions 
// based on those whistles, I need a way to distinguish between them and chose the correct one
//
// Also lookup of the proper function to execute instruction goes through the array of function
// pointers. So I need a way to index into it.
//
// I came up with the next scheme. arm_instruction and thumb_instruction are 
// just funky wrappers around the index. It can be constructed based on the base instruction, 
// which must be passed as a non-type template parametr btw, and a bunch of whistles. 
// 
// Index is assembled from base_offset and relative_offset. Here is an example
// We have next variations of the and_ base instruction:
// and_ and_asr32 and_lsr32 and_rrx and_lsl and_lsr and_asr and_ror and_rslsl and_rslsr and_rsasr and_rsror and_i
// and_s and_asr32s and_lsr32s and_rrxs and_lsls and_lsrs and_asrs and_rors and_rslsls and_rslsrs and_rsasrs and_rsrors and_is
// 
// relative_offset specifies index of the specific instruction in this group relative to the base instruction. So 
// relative_offset of and_ is 0, of and_lsr32 is 2 and so on.
// max_relative_offset<base> specifies amount of the variations of the base so in this case it is equal 26.
// max_absolute_offset<base> specifies the final index of the instruction past the last one in base's group,
// which would be the next base instruction, as of now it is orr.
// max_absolute_offset<base> can be calculated as a sum of all max_absolute_offset of previous instructions as 
// well as current max_relative_offset or in the recursive form:
// max_absolute_offset<base> = max_absolute_offset<previous(base)> + max_relative_offset<base>;
//
// Finally base_offset is an absolute index of the base instruction 
// it can be calculated using max_absolute_offset like this:
// base_offset<base> = max_absolute_offset<previous(base)>;


namespace fgba::cpu {


class arm_instruction {
public:
    enum class set : u32;
    template<set>
    struct flag_bundle;
    template<set Instr>
    using flag_bundle_t = typename flag_bundle<Instr>::type;
    constexpr arm_instruction(std::size_t hash)
        : m_instruction_hash{hash} {}
    template<set>
    [[nodiscard]]static constexpr auto construct(auto... switches) 
        -> arm_instruction;

    [[nodiscard]]constexpr auto as_index() const noexcept 
        -> std::size_t;
    [[nodiscard]]constexpr auto base() const noexcept 
        -> set;
    template<set Instr>
    [[nodiscard]]constexpr auto switches() const noexcept
        -> flag_bundle_t<Instr>;
    [[nodiscard]]static consteval auto count() noexcept 
        -> std::size_t;
private:
    template<set>
    [[nodiscard]]static constexpr auto relative_offset(auto...)
        -> std::size_t;
    template<set>
    inline static constexpr std::size_t max_relative_offset = 1;
    template<set Instr>
    inline static constexpr std::size_t max_absolute_offset = 
        max_absolute_offset<static_cast<set>(std::to_underlying(Instr) - 1)> + max_relative_offset<Instr>;
    template<set Instr>
    static constexpr std::size_t base_offset = max_absolute_offset<static_cast<set>(std::to_underlying(Instr) - 1)>;
private:
    std::size_t m_instruction_hash;
};
class thumb_instruction {
public:
    enum class set : u32;
    constexpr thumb_instruction(set);
    [[nodiscard]]constexpr auto as_index() const noexcept -> size_t;
    [[nodiscard]]constexpr auto handle() const noexcept -> set;
    [[nodiscard]]constexpr auto base() const noexcept -> thumb_instruction;
    [[nodiscard]]static consteval auto count() noexcept -> size_t;
private:
  set m_instruction;
};

[[nodiscard]]auto mask_for(arm_instruction) noexcept -> u32;
[[nodiscard]]auto mask_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto opcode_for(arm_instruction) noexcept -> u32;
[[nodiscard]]auto opcode_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto decode_arm(u32) noexcept -> arm_instruction;
[[nodiscard]]auto decode_thumb(u16) noexcept -> thumb_instruction;

namespace detail {
template<typename... Enums>
inline constexpr u32 offset = ((std::to_underlying(Enums::count)) * ...);
} // namespace detail

enum class arm_instruction::set : u32 {
    b = 0, bx, bl,
    and_, //NOLINT
    orr,
    eor,
    bic,
    add,
    adc,
    sub,
    sbc,
    rsb,
    rsc,
    mov,
    mvn,
    tst,
    teq,
    cmp,
    cmn,
    msr,
    mrs,
    mul,
    mll,
    str,
    ldr,
    undefined,
    
    count,
};

template<arm_instruction::set Instr>
constexpr auto arm_instruction::construct(auto... switches) 
    -> arm_instruction {
    static_assert(
        std::is_same_v<std::tuple<decltype(switches)...>, flag_bundle_t<Instr>>,
        "Wrong instruction flags or their order is incorrect"
    );
    return arm_instruction{base_offset<Instr> + relative_offset<Instr>(switches...)};
}

template<>
inline constexpr std::size_t arm_instruction::max_absolute_offset<arm_instruction::set::b> = 1;
template<>
inline constexpr std::size_t arm_instruction::base_offset<arm_instruction::set::b> = 0;
template<>
struct arm_instruction::flag_bundle<arm_instruction::set::b> { using type = std::tuple<>; };
template<>
constexpr auto arm_instruction::switches<arm_instruction::set::b>() const noexcept
    -> arm_instruction::flag_bundle_t<arm_instruction::set::b> { return {}; }

template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::b>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::b> = 1;
template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::bl>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::bl> = 1;
template<>
struct arm_instruction::flag_bundle<arm_instruction::set::bl> { using type = std::tuple<>; };
template<>
constexpr auto arm_instruction::switches<arm_instruction::set::bl>() const noexcept
    -> arm_instruction::flag_bundle_t<arm_instruction::set::bl> { return {}; }
template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::bx>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::bx> = 1;
template<>
struct arm_instruction::flag_bundle<arm_instruction::set::bx> { using type = std::tuple<>; };
template<>
constexpr auto arm_instruction::switches<arm_instruction::set::bx>() const noexcept
    -> arm_instruction::flag_bundle_t<arm_instruction::set::bx> { return {}; }

template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::and_>(
    immediate_operand im_op,
    shifts shift,
    s_bit s
) -> std::size_t {
    std::size_t s_bit_instructions_offset = 
        static_cast<std::size_t>(std::to_underlying(s)) *
        (detail::offset<shifts> + 1);
    std::size_t shift_or_im_op_offset = 
        im_op == immediate_operand::on ?
            detail::offset<shifts> :
            std::to_underlying(shift);
    return s_bit_instructions_offset + shift_or_im_op_offset;
}
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::and_> = 
    relative_offset<set::and_>(immediate_operand::on, shifts::null, s_bit::on) + 1;

#define FGBA_EXTRACT_SWITCHES_AND_LIKE(Instr)\
template<>\
struct arm_instruction::flag_bundle<arm_instruction::set::Instr> { using type = std::tuple<immediate_operand, shifts, s_bit>; };\
template<>\
constexpr auto arm_instruction::switches<arm_instruction::set::Instr>() const noexcept\
    -> arm_instruction::flag_bundle_t<arm_instruction::set::Instr> {\
    auto relative = m_instruction_hash - base_offset<set::Instr>;\
    auto s_bit_ret = static_cast<s_bit>(relative / (detail::offset<shifts> + 1));\
    relative %= detail::offset<shifts> + 1;\
    auto immediate_operand_ret = static_cast<immediate_operand>(\
        relative == detail::offset<shifts>\
    );\
    if (immediate_operand_ret == immediate_operand::on) {\
        return {immediate_operand_ret, shifts::null, s_bit_ret};\
    } else {\
        return {immediate_operand_ret, static_cast<shifts>(relative), s_bit_ret};\
    }\
}
    

    
#define FGBA_SETUP_OFFSETS(instr)\
template<>\
constexpr auto arm_instruction::relative_offset<arm_instruction::set::instr>(\
    immediate_operand im_op,\
    shifts shift,\
    s_bit s\
) -> std::size_t { return relative_offset<set::and_>(im_op, shift, s); }\
template<>\
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::instr> =\
    relative_offset<set::instr>(immediate_operand::on, shifts::null, s_bit::on) + 1;

FGBA_SETUP_OFFSETS(orr)
FGBA_SETUP_OFFSETS(eor)
FGBA_SETUP_OFFSETS(bic)
FGBA_SETUP_OFFSETS(add)
FGBA_SETUP_OFFSETS(adc)
FGBA_SETUP_OFFSETS(sub)
FGBA_SETUP_OFFSETS(sbc)
FGBA_SETUP_OFFSETS(rsb)
FGBA_SETUP_OFFSETS(rsc)
FGBA_SETUP_OFFSETS(mov)
FGBA_SETUP_OFFSETS(mvn)

#undef FGBA_SETUP_OFFSETS

FGBA_EXTRACT_SWITCHES_AND_LIKE(and_)
FGBA_EXTRACT_SWITCHES_AND_LIKE(orr)
FGBA_EXTRACT_SWITCHES_AND_LIKE(eor)
FGBA_EXTRACT_SWITCHES_AND_LIKE(bic)
FGBA_EXTRACT_SWITCHES_AND_LIKE(add)
FGBA_EXTRACT_SWITCHES_AND_LIKE(adc)
FGBA_EXTRACT_SWITCHES_AND_LIKE(sub)
FGBA_EXTRACT_SWITCHES_AND_LIKE(sbc)
FGBA_EXTRACT_SWITCHES_AND_LIKE(rsb)
FGBA_EXTRACT_SWITCHES_AND_LIKE(rsc)
FGBA_EXTRACT_SWITCHES_AND_LIKE(mov)
FGBA_EXTRACT_SWITCHES_AND_LIKE(mvn)

#undef FGBA_EXTRACT_SWITCHES_AND_LIKE

template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::tst>(
    immediate_operand im_op,
    shifts shift
) -> std::size_t {
    return im_op == immediate_operand::on ? 
        detail::offset<shifts> :
        std::to_underlying(shift);
}
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::tst> =
    relative_offset<set::tst>(immediate_operand::on, shifts::null) + 1;

#define FGBA_EXTRACT_SWITCHES_TST_LIKE(Instr)\
template<>\
struct arm_instruction::flag_bundle<arm_instruction::set::Instr> { using type = std::tuple<immediate_operand, shifts>; };\
template<>\
constexpr auto arm_instruction::switches<arm_instruction::set::Instr>() const noexcept\
    -> arm_instruction::flag_bundle_t<arm_instruction::set::Instr> {\
    auto relative = m_instruction_hash - base_offset<set::Instr>;\
    auto immediate_operand_ret = static_cast<immediate_operand>(\
        relative == detail::offset<shifts>\
    );\
    if (immediate_operand_ret == immediate_operand::on) {\
        return {immediate_operand_ret, shifts::null};\
    } else {\
        return {immediate_operand_ret, static_cast<shifts>(relative)};\
    }\
}

#define FGBA_SETUP_OFFSETS(instr)\
template<>\
constexpr auto arm_instruction::relative_offset<arm_instruction::set::instr>(\
    immediate_operand im_op,\
    shifts shift\
) -> std::size_t { return relative_offset<set::tst>(im_op, shift); }\
template<>\
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::instr> =\
    relative_offset<set::instr>(immediate_operand::on, shifts::null) + 1;

FGBA_SETUP_OFFSETS(teq)
FGBA_SETUP_OFFSETS(cmp)
FGBA_SETUP_OFFSETS(cmn)

#undef FGBA_SETUP_OFFSETS

FGBA_EXTRACT_SWITCHES_TST_LIKE(tst)
FGBA_EXTRACT_SWITCHES_TST_LIKE(teq)
FGBA_EXTRACT_SWITCHES_TST_LIKE(cmp)
FGBA_EXTRACT_SWITCHES_TST_LIKE(cmn)

#undef FGBA_EXTRACT_SWITCHES_TST_LIKE


template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::mrs>(which_psr which_psr)
    -> std::size_t { return std::to_underlying(which_psr); }
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::mrs> = 2;
template<>
struct arm_instruction::flag_bundle<arm_instruction::set::mrs> {
    using type = std::tuple<which_psr>;
};


template<>
constexpr auto arm_instruction::relative_offset<arm_instruction::set::msr>(
    immediate_operand im_op,
    mask mask,
    which_psr which_psr
) -> std::size_t {
    auto psr_offset = std::to_underlying(which_psr) * 3;
    auto mask_offset = mask == mask::on ? 
        std::to_underlying(im_op) :
        2;
    return psr_offset + mask_offset; //NOLINT
}
template<>
inline constexpr std::size_t arm_instruction::max_relative_offset<arm_instruction::set::msr> = 6; 
template<>
struct arm_instruction::flag_bundle<arm_instruction::set::msr> {
    using type = std::tuple<immediate_operand, mask, which_psr>;
};



consteval auto arm_instruction::count() noexcept 
    -> std::size_t { return base_offset<set::count>; }

constexpr auto arm_instruction::as_index() const noexcept 
    -> std::size_t { return m_instruction_hash; }

static_assert(arm_instruction::construct<arm_instruction::set::orr>(immediate_operand::off, shifts::null, s_bit::off).as_index() == 29);

constexpr auto arm_instruction::base() const noexcept 
    -> set {
    return [instruction_hash = this->m_instruction_hash]<std::size_t... Is>(std::index_sequence<Is...>) {
        return static_cast<set>(
            (
                (
                    (
                        instruction_hash < max_absolute_offset<static_cast<set>(Is)> and
                        instruction_hash >= base_offset<static_cast<set>(Is)>
                    ) 
                    * Is
                ) 
                + ...
            )
        );
    }(std::make_index_sequence<static_cast<std::size_t>(set::count)>{});
}
static_assert(arm_instruction::construct<arm_instruction::set::orr>(immediate_operand::off, shifts::null, s_bit::off).base() ==
    arm_instruction::set::orr);
} // namespace fgba::cpu



#endif
