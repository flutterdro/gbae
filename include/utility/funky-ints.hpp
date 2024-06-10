#ifndef FGBA_FUNKY_INTS_HPP_ADJNPLDNC
#define FGBA_FUNKY_INTS_HPP_ADJNPLDNC

#include "fmt/base.h"
#include <bit>
#include <climits>
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <fmt/format.h>
namespace fgba {

using bit = bool;

template<std::unsigned_integral BaseInt>
struct basic_bitset {
    static constexpr unsigned s_max_bit_width = sizeof(BaseInt) * CHAR_BIT;
    struct bit_reference {
        unsigned      bit_index;
        basic_bitset& base;

        constexpr auto operator=(bit bit) noexcept
            -> bit_reference& { 
            base.value &= ~(static_cast<BaseInt>(1) << bit_index); 
            base.value |= static_cast<BaseInt>(bit) << bit_index;
            return *this;
        }
        constexpr operator bit() noexcept { 
            return base.value & (static_cast<BaseInt>(1) << bit_index); 
        }
    };
    struct bit_chunk_reference {
        unsigned      bit_chunk_begin;
        unsigned      bit_chunk_end;
        basic_bitset& base;

        constexpr auto operator=(basic_bitset chunk) noexcept
            -> bit_chunk_reference& { 
            chunk.value &= basic_bitset::create_mask(bit_chunk_end - bit_chunk_begin, 0);
            base.value  &= ~basic_bitset::create_mask(bit_chunk_end, bit_chunk_begin);
            base.value  |= chunk.lsl(bit_chunk_begin).value;

            return *this;
        }
        constexpr auto operator=(bit_chunk_reference& chunk) noexcept
            -> bit_chunk_reference& {
            return this->operator=(static_cast<basic_bitset>(chunk)); //NOLINT
        }
        constexpr operator basic_bitset() noexcept {
            return base.lsr(bit_chunk_begin);
        }

    };

    [[nodiscard]]constexpr auto operator[](unsigned bit_index) const & noexcept
        -> bit { return value & (static_cast<BaseInt>(1) << bit_index); }
    [[nodiscard]]constexpr auto operator[](unsigned bit_index) const && noexcept
        -> bit { return value & (static_cast<BaseInt>(1) << bit_index); }
    [[nodiscard]]constexpr auto operator[](unsigned bit_index) & noexcept
        -> bit_reference { return {.bit_index = bit_index, .base = *this}; }
    [[nodiscard]]constexpr auto operator[](unsigned bit_index) && noexcept
        -> bit { return value & (static_cast<BaseInt>(1) << bit_index); }
    [[nodiscard]]constexpr auto operator[](unsigned bit_chunk_end, unsigned bit_chunk_begin) & noexcept
         -> bit_chunk_reference { 
        return {
            .bit_chunk_begin = bit_chunk_begin, 
            .bit_chunk_end   = bit_chunk_end,
            .base            = *this,
        };
    }
    [[nodiscard]]constexpr auto operator[](unsigned bit_chunk_end, unsigned bit_chunk_begin) const & noexcept
        -> basic_bitset {
        auto mask = create_mask(bit_chunk_end, bit_chunk_begin);
        return {(value & mask) >> bit_chunk_begin};
    }
    [[nodiscard]]constexpr auto operator[](unsigned bit_chunk_end, unsigned bit_chunk_begin) && noexcept
        -> basic_bitset {
        auto mask = create_mask(bit_chunk_end, bit_chunk_begin);
        return {(value & mask) >> bit_chunk_begin};
    }
    [[nodiscard]]constexpr auto operator[](unsigned bit_chunk_end, unsigned bit_chunk_begin) const && noexcept
        -> basic_bitset {
        auto mask = create_mask(bit_chunk_end, bit_chunk_begin);
        return {(value & mask) >> bit_chunk_begin};
    }
    [[nodiscard]]constexpr auto lsl(unsigned amount) const noexcept
        -> basic_bitset { return {value << amount}; }
    [[nodiscard]]constexpr auto asr(unsigned amount) const noexcept
        -> basic_bitset {
        using base_int_but_signed_t = std::make_signed_t<BaseInt>;
        return {
            std::bit_cast<BaseInt>(
                std::bit_cast<base_int_but_signed_t>(value) >> amount
            )
        };
    }
    [[nodiscard]]constexpr auto lsr(unsigned amount) const noexcept
        -> basic_bitset {
        return {value >> amount};
    }
    [[nodiscard]]constexpr auto ror(unsigned amount) const noexcept
        -> basic_bitset { return {std::rotr(value, amount)}; }
    template<unsigned FromBit>
    [[nodiscard]]constexpr auto sign_extend() const noexcept
        -> basic_bitset {
        using base_int_but_signed_t = std::make_signed_t<BaseInt>;
        struct {
            base_int_but_signed_t castrated_value : FromBit + 1;
        } extender;
        
        return { 
            std::bit_cast<BaseInt>(
                static_cast<base_int_but_signed_t>(extender.castrated_value = std::bit_cast<base_int_but_signed_t>(value)) 
            )
        }; 
    }
    [[nodiscard]]constexpr auto sign_extend(unsigned from_bit) const noexcept
        -> basic_bitset { return lsl(s_max_bit_width - 1 - from_bit).asr(s_max_bit_width - 1 - from_bit); }
    [[nodiscard]]constexpr auto mask_out(unsigned end, unsigned begin) const noexcept
        -> basic_bitset { return {value & ~create_mask(end, begin)}; }
    [[nodiscard]]constexpr auto mask_in(unsigned end, unsigned begin) const noexcept
        -> basic_bitset { return {value & create_mask(end, begin)}; }
    [[nodiscard]]constexpr auto byte_swap() const noexcept
        -> basic_bitset { return {std::byteswap(value)}; }
    [[nodiscard]]constexpr auto operator|(basic_bitset other) const noexcept
        -> basic_bitset { return {value | other.value}; }
    [[nodiscard]]constexpr auto operator&(basic_bitset other) const noexcept
        -> basic_bitset { return {value & other.value}; }
    [[nodiscard]]constexpr auto operator^(basic_bitset other) const noexcept
        -> basic_bitset { return {value ^ other.value}; }
    [[nodiscard]]constexpr auto operator+(basic_bitset other) const noexcept
        -> basic_bitset { return {value + other.value}; }
    [[nodiscard]]constexpr auto operator-(basic_bitset other) const noexcept
        -> basic_bitset { return {value - other.value}; }
    [[nodiscard]]constexpr auto operator*(basic_bitset other) const noexcept
        -> basic_bitset { return {value * other.value}; }

    constexpr auto operator|=(basic_bitset other) noexcept
        -> basic_bitset& { value |= other.value; return *this; }
    constexpr auto operator&=(basic_bitset other) noexcept
        -> basic_bitset& { value &= other.value; return *this; }
    constexpr auto operator^=(basic_bitset other) noexcept
        -> basic_bitset& { value ^= other.value; return *this; }
    constexpr auto operator-=(basic_bitset other) noexcept
        -> basic_bitset& { value -= other.value; return *this; }
    constexpr auto operator+=(basic_bitset other) noexcept
        -> basic_bitset& { value += other.value; return *this; }
    
    [[nodiscard]]constexpr auto operator~() const noexcept
        -> basic_bitset { return {~value}; }
    [[nodiscard]]constexpr auto operator-() const noexcept
        -> basic_bitset { return {-value}; }

    [[nodiscard]]constexpr auto operator<=>(basic_bitset const&) const noexcept = default;
    template<typename OtherBaseInt>
    explicit constexpr operator basic_bitset<OtherBaseInt>() const noexcept {
        return {static_cast<OtherBaseInt>(value)}; 
    }
    template<typename T>
    [[nodiscard]]constexpr auto as() const noexcept
        -> T { return static_cast<T>(*this); }
    template<typename T>
    [[nodiscard]]constexpr auto as_exact() const noexcept
        -> T { return std::bit_cast<T>(*this); }
    
    
    [[nodiscard]]static constexpr auto create_mask(unsigned end_bit, unsigned begin_bit)
        -> BaseInt { return (~static_cast<BaseInt>(0) >> (sizeof(BaseInt) * CHAR_BIT - (end_bit - begin_bit) - 1)) << begin_bit; }
    BaseInt value;
};


template<typename T>
using funky = basic_bitset<T>;

consteval auto operator""_bit(unsigned long long num) 
    -> bit { return static_cast<bit>(num); }



} // namespace fgba

consteval auto test1() {
    using t = fgba::funky<std::uint32_t>;
    t tes{0};
    tes[3] = 1;
    tes[4] = 1;
    tes[7, 4] = t{0b1110};
    return tes.value;
}

static_assert(test1());

template<typename T>
struct fmt::formatter<fgba::funky<T>> : fmt::formatter<T> {
    constexpr auto format(fgba::funky<T> value, fmt::format_context& ctx) const {
        return fmt::formatter<T>::format(value.value, ctx);
    }
};

#endif
