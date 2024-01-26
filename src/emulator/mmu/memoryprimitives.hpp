#ifndef MEMORY_PRIMITIVES_HPP_
#define MEMORY_PRIMITIVES_HPP_
#include "cpudefines.hpp"
#include "spdlog/spdlog.h"
#include <__algorithm/ranges_fill.h>
#include <__concepts/invocable.h>
#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <utility>

//owning wrapper around rom
//LowerBound and UpperBound define a range of addresses that this object is responsible for
namespace fgba::mmu {
template<size_t LowerBound, size_t UpperBound>
class readonlymem {
public:
    static constexpr size_t memsize = UpperBound - LowerBound + 1;
    static_assert(memsize % 4 == 0 and LowerBound % 4 == 0, "Memory must be word aligned");
    constexpr auto get_lower_bound() const noexcept -> size_t { return LowerBound; }
    constexpr auto get_upper_bound() const noexcept -> size_t { return UpperBound; }
    readonlymem() { stdr::fill(*m_mem_owner, uninitialized_byte); }
    explicit readonlymem(readonlymem_view<> mem) {
        if (mem.size() != memsize) throw fgba::runtime_error("size missmatch");
        m_mem_owner = std::make_unique<memchunk<memsize>>();
        stdr::copy(mem, m_mem_owner->begin());
    }
    readonlymem(readonlymem&&) noexcept = default;
    auto operator=(readonlymem&&) noexcept 
        -> readonlymem& = default;
    template<size_t Extent = std::dynamic_extent>
    [[nodiscard]]auto get_view_from(u32 address) const noexcept 
        -> readonlymem_view<Extent>{
        auto const offset = address - LowerBound;
        return readonlymem_view<Extent>{m_mem_owner->data() + offset};
    }
    template<typename T>
    [[nodiscard]]auto read(u32 address) const noexcept
        -> T {
        T ret;
        auto const offset = address - LowerBound;
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(T));
        return ret;
    }
private:
    std::unique_ptr<memchunk<memsize>> m_mem_owner;
};
template<size_t LowerBound, size_t UpperBound>
class readwritemem {
public:
    static constexpr size_t memsize = UpperBound - LowerBound + 1;
    static_assert(memsize % 4 == 0 and LowerBound % 4 == 0, "Memory must be word aligned");
    constexpr auto get_lower_bound() const noexcept -> size_t { return LowerBound; }
    constexpr auto get_upper_bound() const noexcept -> size_t { return UpperBound; }
    readwritemem() { stdr::fill(*m_mem_owner, uninitialized_byte); }
    readwritemem(readonlymem_view<> mem) {
        if (mem.size() != memsize) throw fgba::runtime_error("size missmatch");
        m_mem_owner = std::make_unique<memchunk<memsize>>();
        stdr::copy(mem, m_mem_owner->begin());
    }
    readwritemem(readwritemem&&) noexcept = default;
    auto operator=(readwritemem&&) noexcept 
        -> readwritemem& = default;
    template<size_t Extent = std::dynamic_extent>
    [[nodiscard]]auto get_view_from(u32 address) const noexcept 
        -> readonlymem_view<Extent>{
        auto const offset = address - LowerBound;
        return readonlymem_view<Extent>{m_mem_owner->data() + offset};
    }
    template<size_t Extent = std::dynamic_extent>
    [[nodiscard]]auto get_writable_view_from(u32 address) const noexcept 
        -> readonlymem_view<Extent>{
        auto const offset = address - LowerBound;
        return readwritemem_view<Extent>{m_mem_owner->data() + offset};
    }
    template<typename T>
    [[nodiscard]]auto read(u32 address) const noexcept
        -> T {
        T ret;
        auto const offset = address - LowerBound;
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(T));
        return ret;
    }
    template<typename T>
    auto write(u32 address, T data) noexcept
        -> void {
        auto const offset = address - LowerBound;
        std::memcpy(m_mem_owner->data() + offset, &data, sizeof(T));
    }
private:
    std::unique_ptr<memchunk<memsize>> m_mem_owner;
};
//wrapper for memory mapped devices like io registers
template<size_t LowerBound, size_t UpperBound>
class memmap {
public:
    static constexpr size_t memsize = UpperBound - LowerBound + 1;
    static_assert(memsize % 4 == 0 and LowerBound % 4 == 0, "Memory map must be properly aligned");
    constexpr auto get_lower_bound() const noexcept -> size_t { return LowerBound; }
    constexpr auto get_upper_bound() const noexcept -> size_t { return UpperBound; }
    memmap() { 
        for (size_t i = 0; i < memsize; ++i) {
            u32 address = static_cast<u32>(LowerBound + i);
            auto badread = [address] { 
                return memmap::undefined_read(address);
            };
            auto badwrite = [address](u8 data) {
                memmap::undefined_write(address, data);
            };
            (*m_read_callbacks)[i]  = badread;
            (*m_write_callbacks)[i] = badwrite;
        }
    }
    template<typename T> 
    [[nodiscard]]auto read(u32 address) const noexcept
        -> T {
        T ret;
        auto const offset = address - LowerBound;
        std::array<std::byte, sizeof(T)> buf;
        for (size_t i = 0; i < sizeof(T); ++i) {
            buf[i] = std::byte{(*m_read_callbacks)[offset + i]()};
        }
        ret = std::bit_cast<T>(buf);
        return ret;
    }
    template<typename T>
    auto write(u32 address, T data) const noexcept
        -> void {
        auto const offset = address - LowerBound;
        auto buf{
            std::bit_cast<std::array<std::byte, sizeof(T)>>(data)
        };
        for (size_t i = 0; i < sizeof(T); ++i) {
            (*m_write_callbacks)[offset + i](static_cast<u8>(buf[i]));
        }
    }
    template<std::invocable<void> F>
    auto bind_read(u32 address, F&& func) 
        -> void { (*m_read_callbacks)[address - LowerBound] = std::forward<F>(func); }
    template<std::invocable<u8> F>
    auto bind_write(u32 address, F&& func)
        -> void { (*m_write_callbacks)[address - LowerBound] = std::forward<F>(func); }
private:
    [[nodiscard]]static auto undefined_read(u32 address) 
        -> u8 { spdlog::warn("Undefined read at address: {:#010x}", address); return static_cast<u8>(uninitialized_byte);}
    static auto undefined_write(u32 address, u8 data) 
        -> void { spdlog::warn("Undefined write of {1:#x} to address: {0:#010x}", address, data); }
private:
    std::unique_ptr<std::array<std::function<u8(void)>, memsize>> m_read_callbacks;
    std::unique_ptr<std::array<std::function<void(u8)>, memsize>> m_write_callbacks;
};
}
#endif