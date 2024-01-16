#ifndef CPU_DISPLAY_HPP_
#define CPU_DISPLAY_HPP_

#include "../../../emulator/cpu/arm7tdmi.hpp"

namespace fgba::gui {
auto display_cpu_data(cpu::arm7tdmi const&) -> void;

}

#endif