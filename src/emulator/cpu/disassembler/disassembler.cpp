#include "disassembler.hpp"
#include <fmt/core.h>

#include <algorithm>
//NOLINTBEGIN
namespace rngs = std::ranges;
namespace fgba {
[[nodiscard]] auto create_arm_instruction(std::span<std::byte const> instruction_bytes) noexcept
    -> u32;
[[nodiscard]] auto arm_instruction_to_string(u32 instruction)
    -> std::string;
enum arminstr {
    bx = 0,
    b, bl,
    //data processing
    and_, eor, sub, rsb, 
    add, adc, sbc, rsc, tst, teq, cmp, cmn, orr, mov, bic, mvn,
    mrs, msr, msri,
    mul, mla,
    smull, smlal, umull, umlal,
    ldr, str,
    ldrh, strh, ldrsb, ldrsh,
    ldm, stm,
    swp, swpb,
    swi,
    cdp,
    ldc, stc,
    mrc, mcr,
    undefined
};
constexpr std::array<u32, 45> masks{
    //bx
    0x0ffffff0, 
    //b bl
    0x0f000000, 0x0f000000, 
    //data processing
    0x0de00000, 0x0de00000, 0x0de00000, 0x0de00000, 
    0x0de00000, 0x0de00000, 0x0de00000, 0x0de00000,
    0xDF00000, 0xDF00000, 0xDF00000, 0xDF00000,
    0x0de00000, 0x0de00000, 0x0de00000, 0x0de00000,
    //psr transfer
    0xFBF0FFF, 0xFBFFFF0, 0xD3FF000,
    //multiplacation
    0xFF000F0, 0xFF000F0,
    0xFC000F0, 0xFE000F0, 0xF8000F0, 0xFA000F0,
    //load
    0xC100000, 0xC100000,
    0xE500FF0, 0xE500FF0, 0xE500FF0, 0xE500FF0,
    //block data transfer
    0xE100000, 0xE100000,
    //swp swpb
    0xFF00FF0, 0xFF00FF0,
    //swi
    0xF000000,
    //cdp
    0xF000010,
    //ldc stc
    0xE100000, 0xE100000,
    //mrc mcr
    0xF100010, 0xF100010,
    0xE000000
};
constexpr std::array<u32, 45> opcodes{
    //bx
    0x012fff10, 
    //b bl
    0x0a000000, 0x0b000000, 
    //data processing
    0x00000000, 0x00200000, 0x00400000, 0x00600000, 0x00800000, 0x00a00000,
    0x00c00000, 0x00e00000, 0x01100000, 0x01300000, 0x01500000, 0x01700000,
    0x01800000, 0x01a00000, 0x01c00000, 0x01e00000,
    //psr transfer
    0x10F0000, 0x129F000, 0x128F000,
    //multiplication
    0x90, 0x100090,
    0xC00090, 0xE00090, 0x800090, 0xA00090,
    //load
    0x4100000, 0x4000000,
    0x1000B0, 0xB0, 0x1000D0, 0x1000F0,
    //block data transfer
    0x8100000, 0x8000000,
    //swp
    0x1000090, 0x1400090,
    //swi
    0xF000000,
    //cdp
    0xE000000,
    //ldc stc
    0xC100000, 0xC000000,
    //mrc mcr
    0xE100010, 0xE000010,
    0x6000000
};
auto condition_to_string(u32 instruction) -> std::string {
    switch(instruction >> 28) {
        case 0: return "eq";
        case 1: return "ne";
        case 2: return "cs";
        case 3: return "cc";
        case 4: return "mi";
        case 5: return "pl";
        case 6: return "vs";
        case 7: return "vc";
        case 8: return "hi";
        case 9: return "ls";
        case 10: return "ge";
        case 11: return "lt";
        case 12: return "gt";
        case 13: return "le";
        case 14: return "";
        default: return "??";
    }
}
auto register_to_string(u32 registernum) -> std::string {
    std::string ret("r");
    ret += std::to_string(registernum);
    return ret;
}
auto disassemble(std::span<std::byte const> binary) 
    -> std::vector<std::string> {
    std::vector<std::string> ret;

    ret.reserve(binary.size()/4);
    for (size_t offset = 0; binary.size() > offset + 4; offset += 4) {
        u32 const instruction = create_arm_instruction(binary.subspan(offset, 4));
        try {
            auto instr = arm_instruction_to_string(instruction);
            ret.push_back(instr);
        } catch (...) {
            ret.push_back("<undefined>");
        }
    }
    return ret;
}

auto create_arm_instruction(std::span<std::byte const> instruction_bytes) noexcept
    -> u32 {
    std::array<std::byte, 4> buf;
    rngs::copy(instruction_bytes, rngs::begin(buf));
    return std::bit_cast<u32>(buf);
}

auto arm_instruction_to_string(u32 instruction)
    -> std::string {
    std::string ret;
    arminstr instr = bx;
    auto get_operand_2 = [](u32 operand_2, bool imediate_bit) {
        std::string ret;
        if (not imediate_bit) {
            ret = register_to_string(operand_2 & 0xf);
            ret += [shift_type = (operand_2 >> 4) & 1]() -> std::string {
                switch (shift_type) {
                    case 0: return ", lsl ";
                    case 1: return ", lsr ";
                    case 2: return ", asr ";
                    case 3: return ", ror ";
                    default: throw std::runtime_error("");
                }
            }();
            if ((operand_2 >> 4) & 1) {
                ret += register_to_string(operand_2 >> 8 );
            } else {
                ret += '#' + std::to_string(operand_2 >> 7);
            }
        } else {
            ret = "#" + std::to_string((operand_2 & 0xff) << (operand_2 >> 8));
        }

        return ret;
    };
    for (; instr < undefined; instr = static_cast<arminstr>(instr + 1)) {
        if ((instruction & masks[instr]) == opcodes[instr]) break;
    }
    switch (instr) {
//511
    case bx: {
        ret = fmt::format("bx{} {}",
            condition_to_string(instruction),
            register_to_string(instruction & 0xf)
        );
        break;
    }
    case b: {
        ret = fmt::format("b{} {:#x}",
            condition_to_string(instruction),
            (instruction & 0xffffff) << 2
        );
        break;
    }
    case bl:{
        ret = fmt::format("bl{} {:#x}",
            condition_to_string(instruction),
            (instruction & 0xffffff) << 2
        );
        break;
    }
    case and_: {
        ret = fmt::format("and{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case eor: {
        ret = fmt::format("eor{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case sub: {
        ret = fmt::format("sub{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case rsb: {
        ret = fmt::format("rsb{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case add: {
        ret = fmt::format("add{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case adc: {
        ret = fmt::format("adc{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case sbc: {
        ret = fmt::format("sbc{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case rsc: {
        ret = fmt::format("rsc{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case tst: {
        ret = fmt::format("tst{} {}, {}",
            condition_to_string(instruction),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case teq: {
        ret = fmt::format("teq{} {}, {}",
            condition_to_string(instruction),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case cmp: {
        ret = fmt::format("cmp{} {}, {}",
            condition_to_string(instruction),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case cmn: {
        ret = fmt::format("cmn{} {}, {}",
            condition_to_string(instruction),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case orr: {
        ret = fmt::format("orr{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case mov: {
        ret = fmt::format("mov{}{} {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case bic: {
        ret = fmt::format("bic{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case mvn: {
        ret = fmt::format("mvn{}{} {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            get_operand_2(instruction & 0xfff, instruction & (1 << 25))
        );
        break;
    }
    case mrs: {
        ret = fmt::format("mrs{} r{}, {}",
            condition_to_string(instruction),
            (instruction >> 12) & 0xf,
            (instruction >> 22) & 1 ? "spsr" : "cpsr"
        );
        break;
    }
    case msr: {
        ret = fmt::format("msr{} {}, r{}",
            condition_to_string(instruction),
            (instruction >> 22) & 1 ? "spsr" : "cpsr",
            (instruction) & 0xf
        );
        break;
    }
    case msri: {
        auto operand_2 = instruction & 0xfff;
        ret = fmt::format("msr{} {}, {}",
            condition_to_string(instruction),
            (instruction >> 22) & 1 ? "spsr_flg" : "cpsr_flg",
            (instruction >> 25) & 1 ? 
                "#" + std::to_string((operand_2 & 0xff) << (operand_2 >> 8)) :
                register_to_string((instruction) & 0xf)
        );
    }
    case mul: {
        ret = fmt::format("mul{}{} {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 16) & 0xf),
            register_to_string((instruction) & 0xf),
            register_to_string((instruction >> 8) & 0xf)
        );
        break;
    }
    case mla: {
        ret = fmt::format("mla{}{} {}, {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 16) & 0xf),
            register_to_string((instruction) & 0xf),
            register_to_string((instruction >> 8) & 0xf),
            register_to_string((instruction >> 12) & 0xf)
        );
    }
    case smull: {
        ret = fmt::format("smull{}{} {}, {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            register_to_string((instruction) & 0xf),
            register_to_string((instruction >> 8) & 0xf)
        );
        break;
    }
    case smlal: {
        ret = fmt::format("smlal{}{} {}, {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            register_to_string((instruction) & 0xf),
            register_to_string((instruction >> 8) & 0xf)
        );
        break;
    }
    case umull: {
        ret = fmt::format("umull{}{} {}, {}, {}, {}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            register_to_string((instruction >> 12) & 0xf),
            register_to_string((instruction >> 16) & 0xf),
            register_to_string((instruction) & 0xf),
            register_to_string((instruction >> 8) & 0xf)
        );
        break;
    }
    case umlal: {
        ret = fmt::format("umlal{}{} r{}, r{}, r{}, r{}",
            condition_to_string(instruction),
            instruction & (1u << 20) ? "s" : "",
            (instruction >> 12) & 0xf,
            (instruction >> 16) & 0xf,
            (instruction) & 0xf,
            (instruction >> 8) & 0xf
        );
        break;
    }
    case ldr: {
        auto shift = [shift_type = (instruction >> 5) & 0b11]() -> std::string {
            switch (shift_type) {
                case 0: return "lsl";
                case 1: return "lsr";
                case 2: return "asr";
                case 3: return "ror";
                default: return "??";
            }
        }();
        auto offset = (instruction) & (1ul << 25) ? 
            fmt::format("{}r{}, {} #{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf,
                shift,
                (instruction >> 7) & 0b11111) :
            fmt::format("#{}", instruction & 0xfff);
        std::string expresion;
        if ((instruction >> 24) & 1) {
            expresion = fmt::format("[r{}, {}]{}",
                (instruction >> 16) & 0xf,
                offset,
                instruction & (1u << 21) ? "!" : ""
            );
        } else {
            expresion = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset
            );
        }
        ret = fmt::format("ldr{}{} r{}, {}",
            condition_to_string(instruction),
            instruction & (1u << 22) ? "b" : "",
            (instruction >> 12) & 0xf,
            expresion
        );
        break;
    }
    case str: {
        auto shift = [shift_type = (instruction >> 5) & 0b11]() -> std::string {
            switch (shift_type) {
                case 0: return "lsl";
                case 1: return "lsr";
                case 2: return "asr";
                case 3: return "ror";
                default: throw std::runtime_error("");
            }
        }();
        auto offset = (instruction >> 25) & 1 ? 
            fmt::format("{}r{}, {} #{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf,
                shift,
                (instruction >> 7) & 0b11111) :
            "#" + std::to_string(instruction & 0xfff);
        std::string expresion;
        if ((instruction >> 24) & 1) {
            expresion = fmt::format("[r{}, {}]{}",
                (instruction >> 16) & 0xf,
                offset,
                instruction & (1u << 21) ? "!" : ""
            );
        } else {
            expresion = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset
            );
        }
        ret = fmt::format("str{}{} r{}, {}",
            condition_to_string(instruction),
            instruction & (1u << 22) ? "b" : "",
            (instruction >> 12) & 0xf,
            expresion
        );
        break;
    }
    case ldrh: {
        auto address = std::string{};
        auto offset = (instruction >> 22) & 1 ? 
            fmt::format("{}r{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf) :
            "#" + std::to_string(
                (instruction & 0xf) | ((instruction >> 4) & 0xf0)
            );
        if (instruction & (1 << 24)) {
            address = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset 
            );
        } else {
            address = fmt::format("[r{}, {}]",
                (instruction >> 16) & 0xf,
                offset 
            );
        }
        ret = fmt::format("ldr{}h r{}, {}",
            condition_to_string(instruction),
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case strh: {
        auto address = std::string{};
        auto offset = (instruction >> 22) & 1 ? 
            fmt::format("{}r{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf) :
            "#" + std::to_string(
                (instruction & 0xf) | ((instruction >> 4) & 0xf0)
            );
        if (instruction & (1 << 24)) {
            address = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset 
            );
        } else {
            address = fmt::format("[r{}, {}]",
                (instruction >> 16) & 0xf,
                offset 
            );
        }
        ret = fmt::format("str{}h r{}, {}",
            condition_to_string(instruction),
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case ldrsb: {
        auto address = std::string{};
        auto offset = (instruction >> 22) & 1 ? 
            fmt::format("{}r{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf) :
            "#" + std::to_string(
                (instruction & 0xf) | ((instruction >> 4) & 0xf0)
            );
        if (instruction & (1 << 24)) {
            address = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset 
            );
        } else {
            address = fmt::format("[r{}, {}]",
                (instruction >> 16) & 0xf,
                offset 
            );
        }
        ret = fmt::format("ldr{}sb r{}, {}",
            condition_to_string(instruction),
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case ldrsh: {
        auto address = std::string{};
        auto offset = (instruction >> 22) & 1 ? 
            fmt::format("{}r{}",
                (instruction >> 23) & 1 ? "+" : "-",
                instruction & 0xf) :
            "#" + std::to_string(
                (instruction & 0xf) | ((instruction >> 4) & 0xf0)
            );
        if (instruction & (1 << 24)) {
            address = fmt::format("[r{}], {}",
                (instruction >> 16) & 0xf,
                offset 
            );
        } else {
            address = fmt::format("[r{}, {}]",
                (instruction >> 16) & 0xf,
                offset 
            );
        }
        ret = fmt::format("ldr{}sh r{}, {}",
            condition_to_string(instruction),
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case ldm: {
        //TODO: stack ptr case
        auto amn{
            [instruction]() -> std::string {
                std::string ret{};
                ret += instruction & (1 << 23) ? 'i' : 'd';
                ret += instruction & (1 << 24) ? 'b' : 'a';
                return ret;
            }()
        };
        auto rlist{
            [instruction]() -> std::string {
                std::string ret{};
                for (int i = 0; i < 16; ++i) {
                    if (instruction & (1 << i)) {
                        ret += "r" + std::to_string(i) + " ";
                    }
                }
                if (not ret.empty()) ret.pop_back();

                return ret;
            }()
        };
        ret = fmt::format("ldm{}{} r{}{}, {{{}}}{}",
            condition_to_string(instruction),
            amn,
            (instruction >> 16) & 0x0f,
            instruction & (1 << 21) ? "!" : "",
            rlist,
            instruction & (1 << 22) ? "^" : ""
        );
        break;
    }
    case stm:  {
    //TODO: stack ptr case
        auto amn{
            [instruction]() -> std::string {
                std::string ret{};
                ret += instruction & (1 << 23) ? 'i' : 'd';
                ret += instruction & (1 << 24) ? 'b' : 'a';
                return ret;
            }()
        };
        auto rlist{
            [instruction]() -> std::string {
                std::string ret{};
                for (int i = 0; i < 16; ++i) {
                    if (instruction & (1 << i)) {
                        ret += "r" + std::to_string(i) + " ";
                    }
                }
                ret.pop_back();

                return ret;
            }()
        };
        ret = fmt::format("stm{}{} r{}{}, {{{}}}{}",
            condition_to_string(instruction),
            amn,
            (instruction >> 16) & 0x0f,
            instruction & (1 << 21) ? "!" : "",
            rlist,
            instruction & (1 << 22) ? "^" : ""
        );
        break;
    }
    case swp: {
        ret = fmt::format("swp{} r{}, r{}, [r{}]",
            condition_to_string(instruction),
            (instruction >> 12) & 0x0f,
            (instruction) & 0x0f,
            (instruction >> 16) & 0x0f
        );
        break;
    }
    case swpb: {
        ret = fmt::format("swp{}b r{}, r{}, [r{}]",
            condition_to_string(instruction),
            (instruction >> 12) & 0x0f,
            (instruction) & 0x0f,
            (instruction >> 16) & 0x0f
        );
        break;
    }
    case swi: {
        ret = fmt::format("swi{}",
            condition_to_string(instruction)
        );
        break;
    }
    case cdp: {
        ret = fmt::format("cdp{} p{}, {}, c{}, c{}, c{}, {}",
            condition_to_string(instruction),
            (instruction >> 8) & 0xf,
            (instruction >> 20) & 0xf,
            (instruction >> 12) & 0xf,
            (instruction >> 16) & 0xf,
            (instruction) & 0xf,
            (instruction >> 5) & 0x7
        );
        break;
    }
    case ldc: {
        std::string address{
            [instruction](){
                if (instruction & (1 << 24)) {
                    return fmt::format("[r{}], {}",
                        (instruction >> 16) & 0xf,
                        instruction & 0xff
                    );
                } else {
                    return fmt::format("[r{}, {}]{}",
                        (instruction >> 16) & 0xf,
                        instruction & 0xff,
                        instruction & (1 << 21)
                    );
                }
            }()
        };
        ret = fmt::format("ldc{}{} p{}, c{}, {}",
            condition_to_string(instruction),
            instruction & (1 << 22) ? "l" : "",
            (instruction >> 8) & 0xf,
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case stc: {
        std::string address{
            [instruction](){
                if (instruction & (1 << 24)) {
                    return fmt::format("[r{}], {}",
                        (instruction >> 16) & 0xf,
                        instruction & 0xff
                    );
                } else {
                    return fmt::format("[r{}, {}]{}",
                        (instruction >> 16) & 0xf,
                        instruction & 0xff,
                        instruction & (1 << 21)
                    );
                }
            }()
        };
        ret = fmt::format("stc{}{} p{}, c{}, {}",
            condition_to_string(instruction),
            instruction & (1 << 22) ? "l" : "",
            (instruction >> 8) & 0xf,
            (instruction >> 12) & 0xf,
            address
        );
        break;
    }
    case mrc:
        ret = "<undefined>";
        break;
    case mcr:
        ret = "<undefined>";
        break;
    case undefined:
        ret = "<undefined>";
      break;
    }
    return ret; 
}
}
//NOLINTEND