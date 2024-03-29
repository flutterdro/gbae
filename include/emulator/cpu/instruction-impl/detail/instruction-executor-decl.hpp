#ifndef INSTRUCTION_EXECUTION_HPP_DECLARATION_KCNFKJCN
#define INSTRUCTION_EXECUTION_HPP_DECLARATION_KCNFKJCN
#include "emulator/cpu/arm7tdmi.hpp"
#include "emulator/cpudefines.hpp"

namespace fgba::cpu {
//NOLINTBEGIN(cppcoreguidelines-macro-usage)
//disable this check since I couldn't come up with better code generation other than macros
#define DECLARE_ARM_DATA_PROCESSING_VARIATIONS(name)\
    DECLARE_ARM_DATA_PROCESSING_GROUP(name, DECLARE_ARM_DATA_PROCESSING_OPERATION)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION(name##i)
#define DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S(name)\
    DECLARE_ARM_DATA_PROCESSING_GROUP(name, DECLARE_ARM_DATA_PROCESSING_OPERATION_NO_S)\
    DECLARE_ARM_DATA_PROCESSING_OPERATION_NO_S(name##i)
#define DECLARE_ARM_DATA_PROCESSING_GROUP(name, macro)\
    macro(name)\
    macro(name##lsr32)\
    macro(name##asr32)\
    macro(name##rrx)\
    macro(name##lsl)\
    macro(name##lsr)\
    macro(name##asr)\
    macro(name##ror)\
    macro(name##rslsl)\
    macro(name##rslsr)\
    macro(name##rsasr)\
    macro(name##rsror)
#define DECLARE_ARM_DATA_PROCESSING_OPERATION(name) \
    template<bool s>\
    static auto arm_##name(arm7tdmi&, u32) -> void;
#define DECLARE_ARM_DATA_PROCESSING_OPERATION_NO_S(name) \
    static auto arm_##name(arm7tdmi&, u32) -> void;
//NOLINTEND(cppcoreguidelines-macro-usage)
struct instruction_executor {
    static auto arm_bx(arm7tdmi& cpu, u32 instruction) 
        -> void; 
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(and_)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(eor)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(orr)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(bic)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S(tst)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S(teq)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(mov)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(mvn)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(add)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(adc)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(sub)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(sbc)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(rsb)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS(rsc)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S(cmp)
    DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S(cmn)
};
#undef DECLARE_ARM_DATA_PROCESSING_VARIATIONS
#undef DECLARE_ARM_DATA_PROCESSING_VARIATIONS_NO_S
#undef DECLARE_ARM_DATA_PROCESSING_GROUP
#undef DECLARE_ARM_DATA_PROCESSING_OPERATION
#undef DECLARE_ARM_DATA_PROCESSING_OPERATION_NO_S
} //namespace fgba::cpu

#endif