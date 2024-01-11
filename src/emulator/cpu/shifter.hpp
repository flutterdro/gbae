#ifndef SHIFTER_HPP_
#define SHIFTER_HPP_

#include <cassert>
#include <cstdint>
#include <bit>

using u32 = std::uint32_t;
consteval auto operator""_u32(unsigned long long i) 
    -> u32 { return static_cast<u32>(i); }
struct shift_res {
    u32 shifted_data;
    bool carryout;
};
namespace shifter {
enum flags {

};
template<shifter::flags flags>
[[nodiscard]]constexpr auto shift(u32 data, u32 amount, bool carryin) noexcept
    -> shift_res {
    return {0, 0};
}
}


[[nodiscard]] constexpr auto lsl(u32 data, u32 const amount, bool carryin) noexcept
    -> shift_res {
    if (not amount) {
        return {
            .shifted_data = data,
            .carryout = carryin,
        };
    } else if (amount > 32) {
        return {
            .shifted_data = 0,
            .carryout = 0,
        };
    } else if (amount == 32) {
        return {
            .shifted_data = !!(data & 1),
            .carryout = 0,
        };
    } else {
        return {
            .shifted_data = data << amount,
            .carryout = !!(data & (1_u32 << (32 - amount))),
        };
    }
}
[[nodiscard]] constexpr auto lsr(u32 data, u32 amount, bool carryin) noexcept
    -> shift_res {
    if (not amount) {
        return {
            .shifted_data = data,
            .carryout = carryin,
        };
    } 
    if (amount == 32) {
        return {
            .carryout = !!(data & (1_u32 << 31)),
            .shifted_data = 0
        };
    } else if (amount > 32) {
        return {
            .carryout = 0,
            .shifted_data = 0
        };
    } else {
        return {
            .carryout = !!(data & (1_u32 << (amount - 1))),
            .shifted_data = data >> amount
        };
    }
}

[[nodiscard]] constexpr auto asr(u32 data, u32 amount, bool carryin) noexcept 
    -> shift_res {
    if (not amount) {
        return {
            .shifted_data = data,
            .carryout = carryin,
        };
    } 
    if (amount >= 32) {
        return {
            .carryout = !!(data & (1_u32 << 31)),
            .shifted_data = std::bit_cast<u32>(
                std::bit_cast<int32_t>(data & (1_u32 << 31)) >> 31
            ),
        };
    } else {
        return {
            .carryout = !!(data & (1_u32 << (amount - 1))),
            .shifted_data = std::bit_cast<u32>(
                std::bit_cast<int32_t>(data) >> amount
            )
        };
    }
}

[[nodiscard]] constexpr auto ror(u32 data, u32 amount, bool carryin) noexcept 
    -> shift_res {
    if (not amount) {
        return {
            .shifted_data = data,
            .carryout = carryin,
        };
    } 
    amount %= 32;
    if (not amount) {
        return {
            .carryout = !!(data & (1_u32 << 31)),
            .shifted_data = data
        };
    } else {
        return {
            .carryout = !!(data & (1_u32 << (amount - 1))),
            .shifted_data = (data << (32 - amount)) | (data >> amount)
        };
    }
}

[[nodiscard]]constexpr auto ishift(int shift_type, u32 data, u32 amount, bool carryin) noexcept
    -> shift_res {
    if (amount == 0) {
        switch (shift_type) {
            case 0: return lsl(data, 0, carryin);
            case 1: return lsr(data, 32, carryin);
            case 2: return asr(data, 32, carryin);
            case 3: return ror(data, 32, carryin);
        }
    }
    switch (shift_type) {
        case 0: return lsl(data, amount, carryin);
        case 1: return lsr(data, amount, carryin);
        case 2: return asr(data, amount, carryin);
        case 3: return ror(data, amount, carryin);
    }
}
[[nodiscard]]constexpr auto rshift(int shift_type, u32 data, u32 amount, bool carryin) noexcept
    -> shift_res {
    switch (shift_type) {
        case 0: return lsl(data, amount, carryin);
        case 1: return lsr(data, amount, carryin);
        case 2: return asr(data, amount, carryin);
        case 3: return ror(data, amount, carryin);
    }
}
#endif