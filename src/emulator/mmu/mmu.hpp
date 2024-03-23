#ifndef GBA_MMU_H_
#define GBA_MMU_H_
#include <__utility/integer_sequence.h>
#include <__utility/to_underlying.h>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <source_location>
#include <tuple>
#include <type_traits>

#include "../cpudefines.hpp"
#include "../utility/fatexception.hpp"
#include "fmt/format.h"
#include "memoryprimitives.hpp"
#include "io-registers-map.hpp"
#include "ppu/ppu.hpp"

namespace fgba::mmu {
using bios_spec     = mem_spec<bounds<0x00000000, 0x00004000>, mem_type::rom, bus_size::word>;
using ewram_spec    = mem_spec<bounds<0x02000000, 0x02040000>, mem_type::ram, bus_size::hword>;
using iwram_spec    = mem_spec<bounds<0x03000000, 0x03008000>, mem_type::ram, bus_size::word>;
using io_map_spec   = io_registers_map::mem_spec;
using pram_spec     = mem_spec<bounds<0x05000000, 0x05000400>, mem_type::ram, bus_size::hword>;
using vram_spec     = mem_spec<bounds<0x06000000, 0x06018000>, mem_type::ram, bus_size::hword>;
using oam_spec      = mem_spec<bounds<0x07000000, 0x07000400>, mem_type::ram, bus_size::hword>;
using gprom0_spec   = mem_spec<bounds<0x08000000, 0x0a000000>, mem_type::rom, bus_size::hword>;
using gprom1_spec   = mem_spec<bounds<0x0a000000, 0x0c000000>, mem_type::rom, bus_size::hword>;
using gprom2_spec   = mem_spec<bounds<0x0c000000, 0x0e000000>, mem_type::rom, bus_size::hword>;
using sram_spec     = mem_spec<bounds<0x0e000000, 0x0e00ffff>, mem_type::ram, bus_size::byte>;

namespace detail {
template<
    typename Tuple, 
    typename F, 
    typename Indices = std::make_index_sequence<std::tuple_size_v<Tuple>> 
>
struct func_table;
template<typename Tuple, typename F, size_t I>
decltype(auto) apply_for_index(Tuple& tup, F&& func) {
    return std::invoke(std::forward<F>(func), std::get<I>(tup));
}
template<typename Tuple, typename F, size_t... Is>
struct func_table<Tuple, F, std::index_sequence<Is...>> {
    static constexpr std::array table{
        &apply_for_index<Tuple, F, Is>...
    };
};
template<typename Tuple, typename F>
decltype(auto) runtime_get(Tuple& tup, size_t index, F&& func) {
    using tuple_type = std::remove_reference_t<Tuple>;
    return std::invoke(func_table<tuple_type, F>::table[index], tup, std::forward<F>(func));
}
}

template<typename T, typename MemAccessor>
[[nodiscard]] auto read_impl(MemAccessor&& mem, u32 address)
    -> std::optional<T> {
    using spec_type = std::decay_t<MemAccessor>::mem_spec;
    if constexpr (std::same_as<spec_type, sram_spec>) return std::nullopt;
    else {
        if (address >= spec_type::bounds::upper_bound) return std::nullopt;
        if constexpr (sizeof(T) >= std::to_underlying(spec_type::bus_size)) {
            return (mem.template read<half_width_t<T>>(address) << 4 * sizeof(T)) | mem.template read<half_width_t<T>>(address + 2);
        } else {
            return mem.template read<T>(address);
        }
    }
}
class memory_managment_unit {
public:
    memory_managment_unit(ppu::ppu& ppu) 
        : m_mem{
            mem_owner<bios_spec>{}, 
            mem_owner<ewram_spec>{}, 
            mem_owner<iwram_spec>{}, 
            ppu,
            ppu.get_pram(),
            ppu.get_vram(),
            ppu.get_oam(),
            mem_owner<gprom0_spec>{},
            mem_owner<gprom1_spec>{},
            mem_owner<gprom2_spec> {},
            mem_owner<sram_spec>{},
        } {}
    memory_managment_unit(memory_managment_unit const&) = delete;
    auto operator=(memory_managment_unit const&)
        -> memory_managment_unit& = delete;
    memory_managment_unit(memory_managment_unit&&) noexcept = delete;
    auto operator=(memory_managment_unit&&) noexcept
        -> memory_managment_unit& = delete;
    template<typename T>
    [[nodiscard]]auto read(u32 address) const noexcept
        -> std::optional<T> {
        return detail::runtime_get(m_mem, 0, [address](auto&& mem_region) {
            return read_impl<T>(mem_region, address);
        });
    }
    template<typename T>
    auto write(u32 address, T data)
        -> void {
        detail::runtime_get(m_mem, 0, [](auto&& mem_region) {

        });
    }
    auto load_bios(std::filesystem::path const& path) 
        -> void;
    auto load_gamerom(std::filesystem::path const& path_to_cartridge)
        -> void;
private:
    std::tuple<
        mem_owner<bios_spec>, 
        mem_owner<ewram_spec>, 
        mem_owner<iwram_spec>, 
        io_registers_map, 
        mem_view<pram_spec>, 
        mem_view<vram_spec>, 
        mem_view<oam_spec>, 
        mem_owner<gprom0_spec>, 
        mem_owner<gprom1_spec>, 
        mem_owner<gprom2_spec>, 
        mem_owner<sram_spec>
    > m_mem;
};
}

#endif