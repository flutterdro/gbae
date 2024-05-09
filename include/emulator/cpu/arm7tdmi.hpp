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
private:


};
}

#endif
