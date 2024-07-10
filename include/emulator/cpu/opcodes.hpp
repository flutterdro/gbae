#ifndef OP_CODES_HPP_
#define OP_CODES_HPP_

#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/detail/mp_list.hpp"
#include "boost/mp11/list.hpp"
#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
#include "utility/fatexception.hpp"
#include "emulator/cpu/instruction-impl/instruction-flags.hpp"
#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <boost/mp11.hpp>


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
    [[nodiscard]]constexpr auto switches() const noexcept;
    [[nodiscard]]static consteval auto count() noexcept 
        -> std::size_t;
private:
    template<set>
    [[nodiscard]]static constexpr auto relative_offset(auto...);
    template<set>
    [[nodiscard]]static constexpr auto relative_offset(immediate_operand, shifts, s_bit);
    template<set>
    [[nodiscard]]static constexpr auto relative_offset(immediate_operand, shifts);
    template<set Instr>
    static constexpr auto proxy = [](auto... switches) { return relative_offset<Instr>(switches...); };
    template<set Instr>
    using expression_t = decltype(std::apply(proxy<Instr>, std::declval<flag_bundle_t<Instr>>()));
    template<set Instr>
    inline static constexpr std::size_t max_relative_offset = expression_t<Instr>::limit();
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
[[nodiscard]]auto mask_for(arm::instruction_spec) noexcept -> word;
[[nodiscard]]auto mask_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto opcode_for(arm::instruction_spec) noexcept -> word;
[[nodiscard]]auto opcode_for(thumb_instruction) noexcept -> u16;
[[nodiscard]]auto decode(arm::instruction) noexcept -> arm::instruction_spec;
[[nodiscard]]auto decode(thumb::instruction) noexcept -> thumb_instruction;



namespace arm {
namespace detail {
template<typename... Enums>
inline constexpr u32 offset = ((std::to_underlying(Enums::count)) * ...);
} // namespace detail
namespace option_collapsing {
using namespace boost::mp11;
template<auto Val>
using constant = std::integral_constant<decltype(Val), Val>;

template<typename T>
concept limited = requires(T&& val) { 
    {T::limit()}     -> std::same_as<std::size_t>; 
    {val.collapse()} -> std::same_as<std::size_t>;
    T::decompose(std::size_t{});
};

template<auto... Vals>
struct sparse {
    using types = mp_list<decltype(Vals)...>;
    static_assert(mp_same<decltype(Vals)...>::value);
    using option_list = mp_list<mp_first<types>>;
    using options     = mp_rename<option_list, std::tuple>;
    static constexpr auto limit = constant<sizeof...(Vals)>{};
    static constexpr std::array vals{Vals...};
    static constexpr auto decompose(std::size_t composed) noexcept 
        -> options { return {vals[composed]}; }
    constexpr auto collapse() const noexcept {
        return std::ranges::distance(vals.begin(), std::ranges::find(vals, m_value));
    }
    mp_first<types> m_value;
};
template<typename T, typename Limit = constant<T::count>>
struct limiter {
    using option_list = mp_list<T>;
    using options      = std::tuple<T>;
    static constexpr auto decompose(std::size_t composed) noexcept
        -> options {
        return {static_cast<T>(composed)};
    }
    explicit(false) constexpr limiter(T value) : m_value{value} {}
    static constexpr auto limit = constant<static_cast<std::size_t>(Limit{}())>{};
    constexpr auto collapse() const noexcept {
        return static_cast<std::size_t>(m_value);
    }
    T m_value;
};
template<auto E>
using boundary = limiter<decltype(E), constant<E>>;
template<typename T>
using wrap_or_t = mp_eval_if_c<limited<T>, T, limiter, T>;

inline constexpr struct dummy_t {
    using option_list = mp_list<>;
    using options     = std::tuple<>;
    static constexpr auto decompose(std::size_t) noexcept { return options{}; }
    static constexpr auto limit = constant<std::size_t{1}>{};
    constexpr auto collapse() const noexcept { return 0uz; }
} dummy;

template<typename EnumConstant, typename T>
    requires limited<T>
struct match : T {
    static constexpr auto enum_value = EnumConstant{};
    using enum_type                  = typename EnumConstant::value_type;
    constexpr match(EnumConstant, T option) noexcept 
        : T{option} {}
};
template<typename E, typename T>
match(E, T) -> match<E, wrap_or_t<T>>;

template<typename... Ts>
struct matched_bundle {
    constexpr matched_bundle(Ts... elems) noexcept 
        : m_elems{elems...} {}
    std::tuple<Ts...> m_elems;
};

template<typename T>
concept always_false = false;
template<typename T1, typename T2>
struct dependent {
    static_assert(always_false<T1>, "No bueno");
};
template<typename T1, typename T2>
dependent(T1, T2) -> dependent<wrap_or_t<T1>, T2>;

template<typename T, typename... Ts>
    requires limited<T>
struct dependent<T, matched_bundle<Ts...>> {
    static_assert(mp_same<typename Ts::enum_type...>::value, 
        "All types in the matcher must be the same");
    using unique_matchers = mp_size<mp_unique<mp_list<decltype(Ts::enum_value)...>>>;
    static_assert(unique_matchers::value == T::limit(), 
        "Matching must be exhaustive");

    using option_list = mp_set_union<typename T::option_list, typename Ts::option_list...>;
    using options     = mp_rename<option_list, std::tuple>;

    static constexpr auto limit = constant<(Ts::limit() + ...)>{};
    static constexpr auto decompose(std::size_t composed) noexcept {
        auto result = options{};
        ([&]<typename U> -> bool {
            if (composed >= U::limit()) {
                composed -= U::limit();
                return true;
            } else {
                std::get<0>(result) = U::enum_value();
                auto options_that_matter = U::decompose(composed);
                [&]<typename... Us>(std::tuple<Us...> significant_options){
                    ((std::get<Us>(result) = std::get<Us>(significant_options)), ...);
                }(options_that_matter);
                return false;
            }
        }.template operator()<Ts>() and ...);

        return result;
    }

    constexpr auto collapse() const noexcept 
        -> std::size_t {
        auto result = std::size_t{};
        ([&]<typename U> -> bool {
            if (m_value.m_value == U::enum_value()) {
                result += std::get<U>(m_bundle.m_elems).collapse();
                return false;
            } else {
                result += U::limit();
                return true;
            }
        }.template operator()<Ts>() and ...);

        return result;
    } 
    T m_value;
    matched_bundle<Ts...> m_bundle;
};

template<typename T1, typename T2>
    requires (limited<T1> && limited<T2>)
struct independent {
    static constexpr auto limit = constant<T1::limit() * T2::limit()>{};
    using option_list = mp_append<typename T1::option_list, typename T2::option_list>;
    using options     = mp_rename<option_list, std::tuple>;
    static constexpr auto decompose(std::size_t composed) noexcept {
        auto lhs = T1::decompose(composed % T1::limit());
        auto rhs = T2::decompose(composed / T1::limit());
        return std::tuple_cat(lhs, rhs);
    }
    constexpr auto collapse() const noexcept
        -> std::size_t { 
        return m_value1.limit() * m_value2.collapse() + m_value1.collapse(); 
    }

    T1 m_value1;
    T2 m_value2;
};
template<typename T1, typename T2>
independent(T1, T2) -> independent<wrap_or_t<T1>, wrap_or_t<T2>>;

template<typename EnumConst, typename T>
constexpr auto operator|(EnumConst e, T option) noexcept {
    return match{e, option};
}
template<typename T1, typename T2>
constexpr auto operator*(T1 switch1, T2 switch2) noexcept {
    return independent{switch1, switch2};
}
template<typename T, typename... Ts>
constexpr auto operator%(T option, matched_bundle<Ts...> option_bundle) noexcept {
    return dependent{option, option_bundle};
}
template<typename T1, typename T2>
constexpr auto operator,(T1 option1, T2 option2) noexcept {
    return matched_bundle{option1, option2};
}
template<typename T, typename... Ts>
constexpr auto operator,(matched_bundle<Ts...> option_bundle, T option) noexcept {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>){
        return matched_bundle{std::get<Is>(option_bundle.m_elems)..., option};
    }(std::index_sequence_for<Ts...>{});
}
template<typename T, typename... Ts>
constexpr auto operator,(T option, matched_bundle<Ts...> option_bundle) = delete;
}
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
    ldr,
    str,
    stm,
    ldm,
    swp,
    swi,
    coprocessor_placeholder,
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
    return instruction_spec{base_offset<Instr> + relative_offset<Instr>(switches...).collapse()};
}
template<instruction_spec::set Instr>
constexpr auto instruction_spec::switches() const noexcept {
    return expression_t<Instr>::decompose(m_instruction_hash - base_offset<Instr>);
}

template<>
inline constexpr std::size_t instruction_spec::max_absolute_offset<instruction_spec::set::b> = 1;
template<>
inline constexpr std::size_t instruction_spec::base_offset<instruction_spec::set::b> = 0;
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::b> { using type = std::tuple<>; };

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::b>() {
    using namespace option_collapsing;
    return dummy; 
}
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::bl>() { 
    using namespace option_collapsing;
    return dummy; 
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::bl> { using type = std::tuple<>; };
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::bx>() { 
    using namespace option_collapsing;
    return dummy; 
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::bx> { using type = std::tuple<>; };
template<instruction_spec::set Instr>
constexpr auto instruction_spec::relative_offset(
    immediate_operand im_op,
    shifts shift,
    s_bit s
)  {
    using namespace option_collapsing;
    return s * (im_op % (
        constant<immediate_operand::on>{} | dummy,
        constant<immediate_operand::off>{}| shift
    ));
}

#define FGBA_EXTRACT_SWITCHES_AND_LIKE(Instr)\
template<>\
struct instruction_spec::flag_bundle<instruction_spec::set::Instr> { using type = std::tuple<immediate_operand, shifts, s_bit>; };

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

template<instruction_spec::set Instr>
constexpr auto instruction_spec::relative_offset(
    immediate_operand im_op,
    shifts shift
) {
    using namespace option_collapsing;
    return im_op % (
        constant<immediate_operand::on>{} |dummy,
        constant<immediate_operand::off>{}|shift
    );
}

#define FGBA_EXTRACT_SWITCHES_TST_LIKE(Instr)\
template<>\
struct instruction_spec::flag_bundle<instruction_spec::set::Instr> { using type = std::tuple<immediate_operand, shifts>; };\


FGBA_EXTRACT_SWITCHES_TST_LIKE(tst)
FGBA_EXTRACT_SWITCHES_TST_LIKE(teq)
FGBA_EXTRACT_SWITCHES_TST_LIKE(cmp)
FGBA_EXTRACT_SWITCHES_TST_LIKE(cmn)

#undef FGBA_EXTRACT_SWITCHES_TST_LIKE


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mrs>(which_psr which_psr) { 
    using namespace option_collapsing;
    return limiter{which_psr}; 
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mrs> {
    using type = std::tuple<which_psr>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::msr>(
    immediate_operand im_op,
    which_psr which_psr
)  {

    using namespace option_collapsing;
    return which_psr * im_op;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::msr> {
    using type = std::tuple<immediate_operand, which_psr>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mul>(
    s_bit s,
    accumulate a
) {
    using namespace option_collapsing;
    return s * a; //NOLINT
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mul> {
    using type = std::tuple<s_bit, accumulate>;
};


template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::mll>(
    s_bit s,
    accumulate a,
    mll_signedndesd sign
) {
    using namespace option_collapsing;
    return s * a * sign;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::mll> {
    using type = std::tuple<s_bit, accumulate, mll_signedndesd>;
};
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::str>(
    immediate_operand immediate_operand, shifts shift, 
    direction direction, indexing indexing,
    write_back write_back, data_size data_size
) {
    using namespace option_collapsing;
    return (indexing * direction * write_back) * (data_size % (
        constant<data_size::word>{} | (immediate_operand % ( 
            constant<immediate_operand::on>{} |dummy,
            constant<immediate_operand::off>{}|boundary<shifts::rslsl>{shift}
        )),
        constant<data_size::byte>{} | (immediate_operand % ( 
            constant<immediate_operand::on>{} |dummy,
            constant<immediate_operand::off>{}|boundary<shifts::rslsl>{shift}
        )),
        constant<data_size::hword>{}| immediate_operand
    ));
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::str> {
    using type = std::tuple<immediate_operand, shifts, direction, indexing, write_back, data_size>;
};
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::ldr> (
    immediate_operand immediate_operand, shifts shift, 
    direction direction, indexing indexing, write_back write_back, 
    data_size data_size, mll_signedndesd sign
) {
    using namespace option_collapsing;
    return (indexing * direction * write_back) * (data_size % (
        constant<data_size::word>{} |(immediate_operand % (
            constant<immediate_operand::on>{} |dummy,
            constant<immediate_operand::off>{}|boundary<shifts::rslsl>{shift}
        )),
        constant<data_size::hword>{}|immediate_operand * sign,
        constant<data_size::byte> {}|(sign % (
            constant<mll_signedndesd::unsigned_>{}|(immediate_operand % (
                constant<immediate_operand::on>{} |dummy,
                constant<immediate_operand::off>{}|boundary<shifts::rslsl>{shift}
            )),
            constant<mll_signedndesd::signed_>{}  | immediate_operand
        ))
    ));
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::ldr> {
    using type = std::tuple<
    immediate_operand, shifts, 
    direction, indexing, write_back, 
    data_size, mll_signedndesd>;
};

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::stm>(
    s_bit s, direction dir, write_back wb, indexing ind
) {
    using namespace option_collapsing;
    return s * dir * wb * ind;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::stm> {
    using type = std::tuple<s_bit, direction, write_back, indexing>; 
};
template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::ldm>(
    s_bit s, direction dir, write_back wb, indexing ind
) {
    using namespace option_collapsing;
    return s * dir * wb * ind;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::ldm> {
    using type = std::tuple<s_bit, direction, write_back, indexing>; 
};

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::swp>(data_size ds) {
    using namespace option_collapsing;
    return sparse<data_size::byte, data_size::word>{ds};
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::swp> {
    using type = std::tuple<data_size>;
};

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::swi>() {
    using namespace option_collapsing;
    return dummy;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::swi> {
    using type = std::tuple<>;
};

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::undefined>() {
    using namespace option_collapsing;
    return dummy;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::undefined> {
    using type = std::tuple<>;
};

template<>
constexpr auto instruction_spec::relative_offset<instruction_spec::set::coprocessor_placeholder>() {
    using namespace option_collapsing;
    return dummy;
}
template<>
struct instruction_spec::flag_bundle<instruction_spec::set::coprocessor_placeholder> {
    using type = std::tuple<>;
};

consteval auto instruction_spec::count() noexcept 
    -> std::size_t { return base_offset<set::count>; }

constexpr auto instruction_spec::as_index() const noexcept 
    -> std::size_t { return m_instruction_hash; }


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
