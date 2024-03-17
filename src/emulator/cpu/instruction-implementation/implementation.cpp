#include "detail/insctruction-executor-impl.hpp"

namespace fgba::cpu {

namespace  {
#define GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(name)\
    do {\
        GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION_2(name, GEN_ASIGN_IMPL_PTR);\
        GEN_ASIGN_IMPL_PTR(name##i);\
    }while (false)
#define GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION(name)\
    do {\
        GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION_2(name, GEN_ASIGN_IMPL_PTR_NO_S);\
        GEN_ASIGN_IMPL_PTR_NO_S(name##i);\
    }while (false)
#define GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION_2(name, macro)\
    do {\
        macro(name);\
        macro(name##lsr32);\
        macro(name##asr32);\
        macro(name##rrx);\
        macro(name##lsl);\
        macro(name##lsr);\
        macro(name##asr);\
        macro(name##ror);\
        macro(name##rslsl);\
        macro(name##rslsr);\
        macro(name##rsasr);\
        macro(name##rsror);\
    } while(false)
#define GEN_ASIGN_IMPL_PTR(name)\
    do {\
        ret[arm_instruction_set::name] = &instruction_executor::arm_##name<false>;\
        ret[arm_instruction_set::name##s] = &instruction_executor::arm_##name<true>;\
    } while(false)
#define GEN_ASIGN_IMPL_PTR_NO_S(name)\
    do {\
        ret[arm_instruction_set::name] = &instruction_executor::arm_##name;\
    } while(false) 

using impl_ptr = auto (*)(arm7tdmi&, u32) -> void;
constexpr auto init_arm_impl_ptrs() {
    std::array<impl_ptr, arm_instruction_set::undefined> ret{};
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(and_);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(orr);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(eor);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(bic);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(mov);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(mvn);
    GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION(tst);
    GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION(teq);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(add);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(adc);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(sub);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(sbc);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(rsb);
    GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION(rsc);
    GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION(cmp);
    GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION(cmn);
    return ret;
}
#undef GEN_ASIGN_IMPL_PTR
#undef GEN_ASIGN_IMPL_PTR_SHIFT_VARIATION
#undef GEN_ASIGN_IMPL_PTR_SHIFT_AND_S_VARIATION
inline constexpr std::array<impl_ptr, arm_instruction_set::undefined> arm_impl_ptrs = init_arm_impl_ptrs();

}

auto execute_arm(arm7tdmi& cpu, arm_instruction_set instruction, u32 opcode) -> void {
    std::invoke(arm_impl_ptrs[instruction], cpu, opcode);
}

}