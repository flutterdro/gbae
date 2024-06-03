#ifndef FGBA_ARMCPU_HPP
#define FGBA_ARMCPU_HPP

#include "emulator/cpu/bus.hpp"
#include "emulator/cpu/prefetch-buffer.hpp"
#include "emulator/cpu/registermanager.hpp"

namespace fgba::cpu {
class arm7tdmi {
    friend struct instruction_executor;
public:
    arm7tdmi();
    auto connect_bus(connector)
        -> void;
    auto advance_execution()
        -> void;
    [[nodiscard]]auto get_regitsters_contents() const noexcept
        -> register_manager const& {
            return m_registers;
        }
private:
    auto prefetch()
        -> void;
    auto increment_program_counter()
        -> void;
    auto refill_pipeline()
        -> void;
    auto flush_pipeline()
        -> void;
public:
    bus m_bus;
    prefetch_buffer m_prefetch_buffer;
    register_manager m_registers;
private:


};
}

#endif
