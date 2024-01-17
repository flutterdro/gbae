#ifndef FGBA_DISASM_HPP
#define FGBA_DISASM_HPP

#include <span>
#include <vector>
#include <string>

#include "cpudefines.hpp"


namespace fgba {
[[nodiscard]]auto disassemble_arm(std::span<std::byte const>) -> std::vector<std::string>;
[[nodiscard]]auto disassemble_arm(u32) -> std::vector<std::string>;
[[nodiscard]]auto disassemble_thumb(std::span<std::byte const>) -> std::vector<std::string>;
[[nodiscard]]auto disassemble_thumb(u16) -> std::vector<std::string>;


}

#endif