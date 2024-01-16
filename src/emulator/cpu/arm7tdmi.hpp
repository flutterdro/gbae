#ifndef FGBA_ARMCPU_HPP
#define FGBA_ARMCPU_HPP

#include "bus.hpp"
#include "registermanager.hpp"
#include <queue>



namespace fgba::cpu {
class arm7tdmi {
public:
    arm7tdmi();
    auto advance_execution()
        -> void;
    auto execute_arm(u32 instruction) 
        -> void;
    auto execute_thumb(u16 instruction)
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
    std::vector<void(arm7tdmi::*)(u32)> m_arm_impl;
    std::vector<void(arm7tdmi::*)(u16)> m_thumb_impl;
private:
    auto arm_bx      (u32) -> void;
    auto arm_b       (u32) -> void;
    auto arm_bl      (u32) -> void;
    auto arm_and     (u32) -> void;
    auto arm_andi    (u32) -> void;
    auto arm_andrs   (u32) -> void;
};
}

#endif