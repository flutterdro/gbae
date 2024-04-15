#ifndef MEMORY_PRIMITIVES_HPP_
#define MEMORY_PRIMITIVES_HPP_
#include "emulator/cpudefines.hpp"
#include "spdlog/spdlog.h"
#include <__algorithm/ranges_fill.h>
#include <__concepts/invocable.h>
#include "utility/fatexception.hpp"
#include <__ranges/access.h>
#include <__ranges/concepts.h>
#include <__utility/to_underlying.h>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <utility>
#include <ranges>
#include <mdspan>

namespace stdr = std::ranges;
namespace stdv = std::views;
template<size_t LowerBound, size_t UpperBound>
struct bounds {
    static_assert(LowerBound < UpperBound);
    static constexpr size_t lower_bound = LowerBound; 
    static constexpr size_t upper_bound = UpperBound; 
    static constexpr size_t size = upper_bound - lower_bound;
};
enum class mem_type {
    rom,
    ram,
};
enum class bus_size {
    byte  = 1,
    hword = 2,
    word  = 4,
};
template<typename Bounds, mem_type Mem, bus_size Bus>
struct mem_spec {
    using bounds = Bounds;
    static constexpr mem_type mem_type = Mem; 
    static constexpr bus_size bus_size = Bus;
};

constexpr mem_spec<bounds<10, 20>, mem_type::ram, bus_size::word> dummy;
template<typename T>
concept spec = requires {
    T::bounds::lower_bound;
    T::bounds::upper_bound;
    T::mem_type;
    T::bus_size;
};
static_assert(spec<decltype(dummy)>);
namespace fgba::mmu {
template<spec T>
class mem_owner {
public:
    using mem_spec = T;
    mem_owner()
        : m_mem_ptr{std::make_unique<std::array<std::byte, T::bounds::size>>()} {
        stdr::fill(*m_mem_ptr, uninitialized_byte);
    }
    mem_owner(mem_owner&&) noexcept = default;
    auto operator=(mem_owner&&) noexcept
        -> mem_owner& = default;
    template<typename U>
        requires (sizeof(U) <= std::to_underlying(T::bus_size))
    auto read(u32 address) const noexcept 
        -> U {
        assert(address >= T::bounds::lower_bound);
        assert(address <= T::bounds::upper_bound - sizeof(T));
        U res;
        std::memcpy(&res, m_mem_ptr->data() + (address - T::bounds::lower_bound), sizeof(T));
        return res;
    }
    template<typename U>
        requires (sizeof(U) <= std::to_underlying(T::bus_size))
    auto write(u32 address, U data) noexcept 
        -> void requires (T::mem_type == mem_type::ram) {
        assert(address >= T::bounds::lower_bound);
        assert(address <= T::bounds::upper_bound - sizeof(T));
        std::memcpy(m_mem_ptr->data() + (address - T::bounds::lower_bound), &data, sizeof(T));
    }
    [[nodiscard]] constexpr auto begin()        noexcept { return m_mem_ptr->begin();  }
    [[nodiscard]] constexpr auto begin()  const noexcept { return m_mem_ptr->begin();  }
    [[nodiscard]] constexpr auto end()          noexcept { return m_mem_ptr->end();    }
    [[nodiscard]] constexpr auto end()    const noexcept { return m_mem_ptr->end();    }
    [[nodiscard]] constexpr auto cbegin() const noexcept { return m_mem_ptr->cbegin(); }
    [[nodiscard]] constexpr auto cend()   const noexcept { return m_mem_ptr->cend();   }
    [[nodiscard]] constexpr auto data()         noexcept { return m_mem_ptr->data();   }
    [[nodiscard]] constexpr auto data()   const noexcept { return m_mem_ptr->data();   }
private:
    std::unique_ptr<std::array<std::byte, T::bounds::size>> m_mem_ptr;
};

template<spec T>
class mem_view {
public:
    using mem_spec = T;
    mem_view(mem_owner<T> const& mem)
        : m_mem_view{std::span<std::byte, T::bounds::size>{mem}} {}
    mem_view(mem_view const&) noexcept = default;
    auto operator=(mem_view const&) noexcept
        -> mem_view& = default;
    template<typename U>
        requires (sizeof(U) <= std::to_underlying(T::bus_size))
    auto read(u32 address) const noexcept 
        -> U {
        assert(address >= T::bounds::lower_bound);
        assert(address <= T::bounds::upper_bound - sizeof(T));
        U res;
        std::memcpy(&res, m_mem_view.data() + (address - T::bounds::lower_bound), sizeof(T));
        return res;
    }
    template<typename U>
        requires (sizeof(U) <= std::to_underlying(T::bus_size))
    auto write(u32 address, U data) noexcept 
        -> void requires (T::mem_type == mem_type::ram) {
        assert(address >= T::bounds::lower_bound);
        assert(address <= T::bounds::upper_bound - sizeof(T));
        std::memcpy(m_mem_view.data() + (address - T::bounds::lower_bound), &data, sizeof(T));
    }
    [[nodiscard]] constexpr auto begin()        noexcept { return m_mem_view.begin();  }
    [[nodiscard]] constexpr auto begin()  const noexcept { return m_mem_view.begin();  }
    [[nodiscard]] constexpr auto end()          noexcept { return m_mem_view.end();    }
    [[nodiscard]] constexpr auto end()    const noexcept { return m_mem_view.end();    }
    [[nodiscard]] constexpr auto cbegin() const noexcept { return m_mem_view.cbegin(); }
    [[nodiscard]] constexpr auto cend()   const noexcept { return m_mem_view.cend();   }
    [[nodiscard]] constexpr auto data()         noexcept { return m_mem_view.data();   }
    [[nodiscard]] constexpr auto data()   const noexcept { return m_mem_view.data();   }
private:
    std::span<std::byte, T::bounds::size> m_mem_view;
};

template<typename T, stdr::contiguous_range R>
    requires std::same_as<stdr::range_value_t<R>, std::byte>
[[nodiscard]]auto read(R&& range, u32 offset) -> T {
    T res;
    std::memcpy(&res, range.data() + offset, sizeof(T));
    return res;
}

}
#endif
