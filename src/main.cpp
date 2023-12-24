import fstd.vector;
import fgba.cpu.disassembler;
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <print>
#include <ranges>
#include <expected>
namespace stdv = std::ranges::views;
namespace stdr = std::ranges;

int main() try {
    fstd::vector<int> magic{{1,3}};
    auto magicb = magic 
        |stdv::transform([](auto x) { return x*x; })
        |stdv::reverse
        |stdr::to<fstd::vector>();
    std::println("{}", magicb);
    fstd::vector<std::byte> bin{};
    if (std::ifstream file{"../reference/gba_bios.bin", std::ios::binary | std::ios::ate}) {
        auto size = file.tellg();
        bin.resize(size);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(bin.data()), size);
    } else {
        std::println("failed to open a file");
    }
    try {
        fstd::vector<std::string> assembly = fgba::disassemble(bin | std::views::take(300));
        int i = 0;
        for (auto&& line : assembly) {
            std::println("{:0>8x} {:<4} {}", i, "", line);
            i += 4;
        }
    } catch (std::runtime_error& e) {
        std::println("{}", e.what());
    }
    // magic.push_back(5);
    // std::println("adding element:\n{}", magic);
    // magic.emplace_back(2);
    // std::println("emplacing element:\n{}", magic);
    // magic.pop_back();
    // std::println("removing element:\n{}", magic);
    // fstd::vector<unsigned> magic2{};
    // fstd::vector magic3{magic2.begin(), magic2.end()};
    return 0;
} catch (std::runtime_error& e) {
    std::println("{}", e.what());
}