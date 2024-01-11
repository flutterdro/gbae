#ifndef FGBA_DISASM_HPP
#define FGBA_DISASM_HPP

#include <span>
#include <vector>
#include <string>


namespace fgba {
[[nodiscard]]auto disassemble(std::span<std::byte const>) -> std::vector<std::string>;
}

#endif