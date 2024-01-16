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
    constexpr auto is_fiq_disabled() noexcept
        -> bool {
        return val & (1_u32 << 6);
    }
    constexpr auto is_irq_disabled() noexcept
        -> bool {
        return val & (1_u32 << 7);
    }
    constexpr auto get_mode() const noexcept
        -> mode { return static_cast<mode>(val & 0b11111); }
    u32 val;
};

#define asignregisters(mode) do { \
m_active_registers_offset[registers::r0] = registers::r0_ ## mode;\
m_active_registers_offset[registers::r1] = registers::r1_ ## mode;\
m_active_registers_offset[registers::r2] = registers::r2_ ## mode;\
m_active_registers_offset[registers::r3] = registers::r3_ ## mode;\
m_active_registers_offset[registers::r4] = registers::r4_ ## mode;\
m_active_registers_offset[registers::r5] = registers::r5_ ## mode;\
m_active_registers_offset[registers::r6] = registers::r6_ ## mode;\
m_active_registers_offset[registers::r7] = registers::r7_ ## mode;\
m_active_registers_offset[registers::r8] = registers::r8_ ## mode;\
m_active_registers_offset[registers::r9] = registers::r9_ ## mode;\
m_active_registers_offset[registers::r10] = registers::r10_ ## mode;\
m_active_registers_offset[registers::r11] = registers::r11_ ## mode;\
m_active_registers_offset[registers::r12] = registers::r12_ ## mode;\
m_active_registers_offset[registers::r13] = registers::r13_ ## mode;\
m_active_registers_offset[registers::lr] = registers::lr_ ## mode;\
m_active_registers_offset[registers::pc] = registers::pc_ ## mode;\
} while(false)

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
            case mode::usr: asignregisters(usr); break;
            case mode::sys: asignregisters(sys); break;
            case mode::fiq: asignregisters(fiq); break;
            case mode::irq: asignregisters(irq); break;
            case mode::svc: asignregisters(svc); break;
            case mode::abt: asignregisters(abt); break;
            case mode::und: asignregisters(und); break;
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
    [[nodiscard]]constexpr auto cpsr() const noexcept
        -> psr const& { return m_cpsr; }
private:
    psr m_cpsr; 
    std::array<u32, 31> m_register_bank;
    std::array<psr, 5> m_spsr;
    std::array<unsigned, 16> m_active_registers_offset;
};

}

#endif