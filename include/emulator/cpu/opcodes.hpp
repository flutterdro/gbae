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
// I came up with the next scheme. instruction_spec and thumb_instruction are 
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

namespace arm {
class instruction_spec {
public:
    enum class set : u32;
    template<set>
    struct flag_bundle;
    template<set Instr>
    using flag_bundle_t = typename flag_bundle<Instr>::type;
    constexpr instruction_spec(std::size_t hash)
        : m_instruction_hash{hash} {}
    template<set>
    [[nodiscard]]static constexpr auto construct(auto... switches) 
        -> instruction_spec;

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


} // namespace arm

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
[[nodiscard]]auto mask_for(arm::instruction_spec) noexcept -> u32;
[[nodiscard]]auto mask_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto opcode_for(arm::instruction_spec) noexcept -> u32;
[[nodiscard]]auto opcode_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto decode(arm::instruction) noexcept -> arm::instruction_spec;
[[nodiscard]]auto decode(thumb::instruction) noexcept -> thumb_instruction;

namespace detail {
template<typename... Enums>
inline constexpr u32 offset = ((std::to_underlying(Enums::count)) * ...);
} // namespace detail

namespace arm {

enum class instruction_spec::set : u32 {
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

template<instruction_spec::set Instr>
constexpr auto instruction_spec::construct(auto... switches) 
    -> instruction_spec {
    static_assert(
        std::is_same_v<std::tuple<decltype(switches)...>, flag_bundle_t<Instr>>,
        "Wrong instruction flags or their order is incorrect"
    );
    return instruction_spec{base_offset<Instr> + relative_offset<Instr>(switches...)};
}

template<>
inline constexpr std::size_t instruction_spec::max_absolute_offset<instruction_spec::set::b> = 1;
template<>
inline constexpr std::size_t instruction_spec::base_offset<instruction_spec::set::b> = 0;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::b> { using type = std::tuple<>; };
template<>
constexpr auto instruction_spec::switches<instruction_spec::set::b>() const noexcept
    -> instruction_spec::flag_bundle_t<instruction_spec::set::b> { return {}; }

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::b>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::b> = 1;
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::bl>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::bl> = 1;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::bl> { using type = std::tuple<>; };
template<>
constexpr auto instruction_spec::switches<instruction_spec::set::bl>() const noexcept
    -> instruction_spec::flag_bundle_t<instruction_spec::set::bl> { return {}; }
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::bx>()
    -> std::size_t { return 0; }
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::bx> = 1;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::bx> { using type = std::tuple<>; };
template<>
constexpr auto instruction_spec::switches<instruction_spec::set::bx>() const noexcept
    -> instruction_spec::flag_bundle_t<instruction_spec::set::bx> { return {}; }

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::and_>(
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
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::and_> = 
    relative_offset<set::and_>(immediate_operand::on, shifts::null, s_bit::on) + 1;

#define FGBA_EXTRACT_SWITCHES_AND_LIKE(Instr)\
template<>\
struct instruction_spec::flag_bundle<instruction_spec::set::Instr> { using type = std::tuple<immediate_operand, shifts, s_bit>; };\
template<>\
constexpr auto instruction_spec::switches<instruction_spec::set::Instr>() const noexcept\
    -> instruction_spec::flag_bundle_t<instruction_spec::set::Instr> {\
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
constexpr auto instruction_spec::relative_offset<instruction_spec::set::instr>(\
    immediate_operand im_op,\
    shifts shift,\
    s_bit s\
) -> std::size_t { return relative_offset<set::and_>(im_op, shift, s); }\
template<>\
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::instr> =\
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
constexpr auto instruction_spec::relative_offset<instruction_spec::set::tst>(
    immediate_operand im_op,
    shifts shift
) -> std::size_t {
    return im_op == immediate_operand::on ? 
        detail::offset<shifts> :
        std::to_underlying(shift);
}
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::tst> =
    relative_offset<set::tst>(immediate_operand::on, shifts::null) + 1;

#define FGBA_EXTRACT_SWITCHES_TST_LIKE(Instr)\
template<>\
struct instruction_spec::flag_bundle<instruction_spec::set::Instr> { using type = std::tuple<immediate_operand, shifts>; };\
template<>\
constexpr auto instruction_spec::switches<instruction_spec::set::Instr>() const noexcept\
    -> instruction_spec::flag_bundle_t<instruction_spec::set::Instr> {\
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
constexpr auto instruction_spec::relative_offset<instruction_spec::set::instr>(\
    immediate_operand im_op,\
    shifts shift\
) -> std::size_t { return relative_offset<set::tst>(im_op, shift); }\
template<>\
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::instr> =\
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
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mrs>(which_psr which_psr)
    -> std::size_t { return std::to_underlying(which_psr); }
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::mrs> = 2;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mrs> {
    using type = std::tuple<which_psr>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::msr>(
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
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::msr> = 6; 
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::msr> {
    using type = std::tuple<immediate_operand, mask, which_psr>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mul>(
    s_bit s,
    accumulate a
) -> std::size_t {
    return detail::offset<s_bit> * std::to_underlying(a) + std::to_underlying(s); //NOLINT
}
template<>
inline constexpr std::size_t instruction_spec::max_relative_offset<instruction_spec::set::mul> = 4;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mul> {
    using type = std::tuple<s_bit, accumulate>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mll>(
    s_bit s,
    accumulate a,
    mll_signedndesd sign
) -> std::size_t {
    auto tu = [](auto val) -> std::size_t { return std::to_underlying(val); }; //NOLINT
    return detail::offset<s_bit, accumulate> * tu(sign) + detail::offset<s_bit> * tu(a) + tu(s);
}
template<>
inline constexpr auto instruction_spec::max_relative_offset<instruction_spec::set::mll> = 8;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mll> {
    using type = std::tuple<s_bit, accumulate, mll_signedndesd>;
};

consteval auto instruction_spec::count() noexcept 
    -> std::size_t { return base_offset<set::count>; }

constexpr auto instruction_spec::as_index() const noexcept 
    -> std::size_t { return m_instruction_hash; }

static_assert(instruction_spec::construct<instruction_spec::set::orr>(immediate_operand::off, shifts::null, s_bit::off).as_index() == 29);

constexpr auto instruction_spec::base() const noexcept 
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
static_assert(instruction_spec::construct<instruction_spec::set::orr>(immediate_operand::off, shifts::null, s_bit::off).base() ==
    instruction_spec::set::orr);

} // namespace arm

} // namespace fgba::cpu



#endif
