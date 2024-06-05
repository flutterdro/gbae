#ifndef FGBA_FUNKY_INTS_HPP_ADJNPLDNC
#define FGBA_FUNKY_INTS_HPP_ADJNPLDNC

#include <bit>
#include <climits>
#include <concepts>
#include <cstdint>
#include <type_traits>
namespace fgba {

using bit = bool;

template<std::unsigned_integral BaseInt>
struct basic_bitset {
    struct bit_reference {
        unsigned      bit_index;
        basic_bitset& base;

        constexpr auto operator=(bit value) noexcept
            -> bit_reference& { 
            base.value &= ~(static_cast<BaseInt>(1) << bit_index); 
            base.value |= static_cast<BaseInt>(value) << bit_index;
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
            chunk.value &= ~basic_bitset::create_mask(bit_chunk_end - bit_chunk_begin, 0);
            base.value  &= ~basic_bitset::create_mask(bit_chunk_end, bit_chunk_begin);
            base.value  |= chunk.lsl(bit_chunk_begin).value;

            return *this;
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
        return { extender.castrated_value = value }; 
    }
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

    [[nodiscard]]constexpr auto operator|=(basic_bitset other) noexcept
        -> basic_bitset& { value |= other.value; return *this; }
    [[nodiscard]]constexpr auto operator&=(basic_bitset other) noexcept
        -> basic_bitset& { value &= other.value; return *this; }
    [[nodiscard]]constexpr auto operator^=(basic_bitset other) noexcept
        -> basic_bitset& { value ^= other.value; return *this; }
    [[nodiscard]]constexpr auto operator-=(basic_bitset other) noexcept
        -> basic_bitset& { value -= other.value; return *this; }
    [[nodiscard]]constexpr auto operator+=(basic_bitset other) noexcept
        -> basic_bitset& { value += other.value; return *this; }
    
    [[nodiscard]]constexpr auto operator~() const noexcept
        -> basic_bitset { return {~value}; }
    [[nodiscard]]constexpr auto operator-() const noexcept
        -> basic_bitset { return {-value}; }
    template<typename T>
    [[nodiscard]]constexpr auto as() const noexcept
        -> T { return static_cast<T>(*this); }
    template<typename T>
    [[nodiscard]]constexpr auto as_exact() const noexcept
        -> T { return std::bit_cast<T>(*this); }
    template<typename OtherBaseInt>
    explicit constexpr operator basic_bitset<OtherBaseInt>() {
        return {static_cast<OtherBaseInt>(value)}; 
    }
    [[nodiscard]]static constexpr auto create_mask(unsigned end_bit, unsigned begin_bit)
        -> BaseInt { return (~static_cast<BaseInt>(0) >> (sizeof(BaseInt) * CHAR_BIT - end_bit)) << begin_bit; }
    BaseInt value;
};


template<typename T>
using funky = basic_bitset<T>;

consteval auto operator""_bit(unsigned long long num) 
    -> bit { return static_cast<bit>(num); }

} // namespace fgba


#endif