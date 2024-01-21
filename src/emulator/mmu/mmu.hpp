#ifndef GBA_MMU_H_
#define GBA_MMU_H_
#include <cstddef>
#include <fstream>
#include <iostream>
#include <source_location>

#include "../cpudefines.hpp"
#include "../utility/fatexception.hpp"

namespace fgba::mmu {
enum class memory_region_type : u32 {
    sysrom = 0x00,
    ewram,
    iwram,
    ioregisters,
    //internal display memory
    paletteram,
    vram,
    oam,
    //external memory (game pak)
    flashrom0,
    flashrom1,
    flashrom2,
    sram,
    invalid,
};
class memory_managment_unit {
public:
    memory_managment_unit();
    memory_managment_unit(memory_managment_unit const&) = delete;
    auto operator=(memory_managment_unit const&)
        -> memory_managment_unit& = delete;
    memory_managment_unit(memory_managment_unit&&) noexcept = default;
    auto operator=(memory_managment_unit&&) noexcept
        -> memory_managment_unit& = default;

    template<size_t bus_size>
    [[nodiscard]]auto read(address) const
        -> bus_data<bus_size>;
    template<size_t bus_size>
    auto write(u32 address, bus_data<bus_size> data)
        -> void;

    auto load_bios(std::filesystem::path const& path) 
        -> void {
        if (std::ifstream file{path, std::ios::binary | std::ios::ate}) {
            auto& sys_rom{m_memory[0]};
            auto size{file.tellg()};
            if (sys_rom.size() < static_cast<size_t>(size)) {
                throw fgba::runtime_error{"binary size mismatch", std::source_location::current()};
            }
            file.seekg(0);
            file.read(reinterpret_cast<char*>(sys_rom.data()), size);
        } else {
            throw fgba::runtime_error{"couldn't open a file", std::source_location::current()};
        }
    }
    auto load_gamerom(std::filesystem::path const& path_to_cartridge)
        -> void;
private:
    std::array<mem_region, 11> m_memory;
};
}

#endif