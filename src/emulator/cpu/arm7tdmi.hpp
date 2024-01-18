#ifndef FGBA_ARMCPU_HPP
#define FGBA_ARMCPU_HPP

#include "bus.hpp"
#include "registermanager.hpp"
#include <queue>

#define DECLARE_ARM_DATA_PROCESSING_GROUP(name)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##lsl0)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##lsr32)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##asr32)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##rrx)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##lsl)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##lsr)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##asr)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##ror)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##rslsl)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##rslsr)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##rsasr)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##rsror)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##i)
#define DECLARE_ARM_DATA_PROCESSING_OPERATION(name) \
    auto arm_##name(u32) noexcept -> void;
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
    DECLARE_ARM_DATA_PROCESSING_GROUP(and_);
    DECLARE_ARM_DATA_PROCESSING_GROUP(and_s);
    DECLARE_ARM_DATA_PROCESSING_GROUP(eor);
    DECLARE_ARM_DATA_PROCESSING_GROUP(eors);
    DECLARE_ARM_DATA_PROCESSING_GROUP(orr);
    DECLARE_ARM_DATA_PROCESSING_GROUP(orrs);
    DECLARE_ARM_DATA_PROCESSING_GROUP(bic);
    DECLARE_ARM_DATA_PROCESSING_GROUP(bics);
    DECLARE_ARM_DATA_PROCESSING_GROUP(tst);
    DECLARE_ARM_DATA_PROCESSING_GROUP(teq);
    DECLARE_ARM_DATA_PROCESSING_GROUP(mov);
    DECLARE_ARM_DATA_PROCESSING_GROUP(movs);
    DECLARE_ARM_DATA_PROCESSING_GROUP(movn);
    DECLARE_ARM_DATA_PROCESSING_GROUP(movns);
};
}

#endif