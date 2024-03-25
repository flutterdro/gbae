#ifndef FGBA_ARMCPU_HPP
#define FGBA_ARMCPU_HPP

#include "emulator/cpu/bus.hpp"
#include "emulator/cpu/registermanager.hpp"
#include <queue>

namespace fgba::cpu {
class arm7tdmi {
    friend struct instruction_executor;
public:
    arm7tdmi();
    auto advance_execution()
        -> void;
    auto execute_arm(u32 instruction) 
        -> void;
    auto execute_thumb(u16 instruction)
        -> void;
    auto interrupt_arm(u32 instruction) 
        -> void;
    auto interrupt_thumb(u16 instruction)
        -> void;
    template<std::invocable<bus::signals> F>
    auto conect_read(F&& func) 
        -> void {
        m_bus.conect_read(std::forward<F>(func));
    }
    template<std::invocable<u32, bus::signals> F>
    auto conect_write(F&& func)
        -> void {
        m_bus.conect_write(std::forward<F>(func));
    }
    [[nodiscard]]auto get_regitsters_contents() const noexcept
        -> register_manager const& {
            return m_registers;
        }
private:
    auto prefetch()
        -> void;
    auto flush()
        -> void;
public:
    bus m_bus;
    std::queue<u32> m_prefetch_buffer;
    register_manager m_registers;
    std::vector<void(arm7tdmi::*)(u16)> m_thumb_impl;
private:


};
}

#endif