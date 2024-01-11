#ifndef REGISTER_MANAGER_HPP_
#define REGISTER_MANAGER_HPP_

#include <array>
#include <cstddef>
#include <cassert>

#include "../defines.hpp"

namespace fgba::cpu {

struct psr {
    constexpr auto set_ccf(ccf const flag, bool b) noexcept
        -> void {
        val = (val & (~flag)) | (flag * b);
    } 
    constexpr auto set_ccf(ccf const flag) noexcept
        -> void {
        val |= flag; 
    } 
    constexpr auto reset_ccf(ccf const flag) noexcept
        -> void {
        val &= ~flag; 
    }
    constexpr auto check_ccf(ccf const flag) noexcept
        -> bool {
        return val & flag;
    }
    constexpr auto fiq_disable(bool b) noexcept
        -> void {
        val = (val & ~(1_u32 << 6)) | ((1_u32 << 6) * b);
    }
    constexpr auto irq_disable(bool b) noexcept
        -> void {
        val = (val & ~(1_u32 << 7)) | ((1_u32 << 7) * b);
    }
    constexpr auto use_thumb(bool b) noexcept
        -> void {
        val = (val & ~(1_u32 << 5)) | ((1_u32 << 5) * b);
    }
    constexpr auto is_thumb() noexcept
        -> bool {
        return val & (1_u32 << 5);
    }
    u32 val;
};

class register_manager {
public:
    constexpr register_manager() noexcept {
        for (size_t i = 0; i < 16; ++i) {
            m_active_registers_offset[i] = i;
        }
    }
    constexpr auto switch_mode(mode const mode) noexcept
        -> void {
        switch (mode) {
            case mode::usr: [[fallthrough]];
            case mode::sys: {
                for (size_t i = 8; i < 15; ++i) {
                    m_active_registers_offset[i] = i;
                }
                break;
            }
            case mode::fiq: {
                for (size_t i = 8; i < 15; ++i) {
                    m_active_registers_offset[i] = i + 8;
                }
                break;
            }
            case mode::irq: {
                for (size_t i = 8; i < 13; ++i) {
                    m_active_registers_offset[i] = i;
                }
                m_active_registers_offset[13] = 23;
                m_active_registers_offset[14] = 24;
                break;
            }
            case mode::svc: {
                for (size_t i = 8; i < 13; ++i) {
                    m_active_registers_offset[i] = i;
                }
                m_active_registers_offset[13] = 25;
                m_active_registers_offset[14] = 26;
                break;
            }
            case mode::abt: {
                for (size_t i = 8; i < 13; ++i) {
                    m_active_registers_offset[i] = i;
                }
                m_active_registers_offset[13] = 27;
                m_active_registers_offset[14] = 28;
                break;
            }
            case mode::und: {
                for (size_t i = 8; i < 13; ++i) {
                    m_active_registers_offset[i] = i;
                }
                m_active_registers_offset[13] = 29;
                m_active_registers_offset[14] = 30;
                break;
            }
            default: assert(false);
        } 
    }
    [[nodiscard]]constexpr auto operator[](size_t const index) noexcept
        -> u32& {
        return m_register_bank[m_active_registers_offset[index]];
    }
    [[nodiscard]]constexpr auto operator[](size_t const index) const noexcept
        -> u32 const& {
        return m_register_bank[m_active_registers_offset[index]];
    }
    [[nodiscard]]constexpr auto pc() noexcept
        -> u32& { return m_register_bank[15]; }
    [[nodiscard]]constexpr auto cpsr() noexcept
        -> psr& { return m_cpsr; }
private:
    psr m_cpsr; 
    std::array<u32, 31> m_register_bank;
    std::array<psr, 5> m_spsr;
    std::array<unsigned, 16> m_active_registers_offset;
};

}

#endif