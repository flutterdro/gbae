#include "emulator/cpu/opcodes.hpp"

namespace fgba::cpu {

enum class set {
    b, bl, bx,

    and_, orr, eor, bic,
    add, adc, sub, sbc, rsb, rsc,
    tst, teq, cmp, cmn,
    mov, mvn,

    undefined,

    size,
};

}
