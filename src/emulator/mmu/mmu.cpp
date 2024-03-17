#include "mmu.hpp"
#include "cpudefines.hpp"
#include "fatexception.hpp"
#include "mmu/memoryprimitives.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <ranges>
#include <algorithm>
#include <source_location>


namespace fgba::mmu {
// constexpr std::array<int, 16> internal_region_mapping{
//     0, -1, 1, 2, 3, 4, 5, 6, 7, 7, 8, 8, 9, 9, 10, -1 
// };

// auto get_region_type(address address) 
//     -> memory_region_type {
//     using enum memory_region_type;
//     constexpr std::array<memory_region_type, 16> region_mapping {
//         sysrom, invalid, ewram, iwram, ioregisters, paletteram, 
//         vram, oam, flashrom0, flashrom0, flashrom1, flashrom1,
//         flashrom2, flashrom2, sram, invalid, 
//     };
//     return region_mapping[(address >> 24) & 0xf];
// }

struct bounds {
    u32 upper;
    u32 lower;
};

// constexpr std::array<bounds, 11> boundaries {
//     bounds{0x00003FFF, 0x00000000}, bounds{0x0203FFFF, 0x02000000}, 
//     bounds{0x03007FFF, 0x03000000}, bounds{0x040003FE, 0x04000000}, 

//     bounds{0x050003FF, 0x05000000}, bounds{0x06017FFF, 0x06000000}, bounds{0x070003FF, 0x07000000},

//     bounds{0x09FFFFFF, 0x08000000}, bounds{0x0BFFFFFF, 0x0A000000}, 
//     bounds{0x0DFFFFFF, 0x0C000000}, bounds{0x0E00FFFF, 0x0E000000}
// };

// auto memory_managment_unit::load_bios(readonlymem_view<> biosbin) 
//     -> void {
//     m_bios = readonlymem<0x00000000, 0x00003fff>(biosbin);
// }


}