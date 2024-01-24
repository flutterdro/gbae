#ifndef MEMORY_PRIMITIVES_HPP_
#define MEMORY_PRIMITIVES_HPP_
#include "cpudefines.hpp"
#include "spdlog/spdlog.h"
#include <__algorithm/ranges_fill.h>
#include <__concepts/invocable.h>
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
    readonlymem() { stdr::fill(*m_mem_owner, uninitialized_byte); }
    readonlymem(readonlymem_view<> mem) {
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
    [[nodiscard]]auto read_word(u32 address) const noexcept 
        -> u32 { 
        u32 ret;
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 4);
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(u32));
        return ret;
    }
    [[nodiscard]]auto read_halfword(u32 address) const noexcept
        -> u16 {
        u16 ret;
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 2);
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(u16));
        return ret;
    }
    [[nodiscard]]auto read_byte(u32 address) const noexcept 
        -> u8 {
        return (*m_mem_owner)[address - LowerBound];
    }
private:
    std::unique_ptr<memchunk<memsize>> m_mem_owner;
};
template<size_t LowerBound, size_t UpperBound>
class readwritemem {
public:
    static constexpr size_t memsize = UpperBound - LowerBound + 1;
    static_assert(memsize % 4 == 0 and LowerBound % 4 == 0, "Memory must be word aligned");
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
    [[nodiscard]]auto read_word(u32 address) const noexcept 
        -> u32 { 
        u32 ret;
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 4);
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(u32));
        return ret;
    }
    auto write_word(u32 address, u32 data) noexcept
        -> void {
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 4);
        std::memcpy(m_mem_owner->data(), &data, sizeof(u32));
    }
    [[nodiscard]]auto read_halfword(u32 address) const noexcept
        -> u16 {
        u16 ret;
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 2);
        std::memcpy(&ret, m_mem_owner->data() + offset, sizeof(u16));
        return ret;
    }
    auto write_halfword(u32 address, u16 data) noexcept
        -> void {
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound - 2);
        std::memcpy(m_mem_owner->data(), &data, sizeof(u16));
    }
    [[nodiscard]]auto read_byte(u32 address) const noexcept 
        -> u8 {
        return (*m_mem_owner)[address - LowerBound];
    }
    auto write_byte(u32 address, u8 data) noexcept
        -> void {
        auto const offset = address - LowerBound;
        assert(offset <= UpperBound);
        (*m_mem_owner)[offset] = data;
    }
private:
    std::unique_ptr<memchunk<memsize>> m_mem_owner;
};
//wrapper for memory mapped devices like io registers
template<size_t LowerBound, size_t UpperBound>
class memmap {
static_assert(UpperBound % 2 == 0 and LowerBound % 2 == 0, "Memory map must be halfword aligned");
public:
    static constexpr size_t memsize = UpperBound - LowerBound + 1;
    memmap() { 
        for (size_t i = 0; i < memsize / 2; ++i) {
            auto badread = [i] { 
                return memmap::undefined_read(i * 2 + LowerBound);
            };
            auto badwrite = [i](u16 data) {
                memmap::undefined_write(i * 2 + LowerBound, data);
            };
            (*m_read_callbacks)[i]  = badread;
            (*m_write_callbacks)[i] = badwrite;
        }
    }
    [[nodiscard]]auto read(u32 address) noexcept
        -> u16 { return (*m_read_callbacks)[(address - LowerBound) / 2](); }
    auto write(u32 address, u16 data) noexcept 
        -> void { (*m_write_callbacks)[(address - LowerBound) / 2](data); }
    template<std::invocable<void> F>
    auto bind_read(u32 address, F&& func) 
        -> void { (*m_read_callbacks)[(address - LowerBound) / 2] = std::forward<F>(func); }
    template<std::invocable<u16> F>
    auto bind_write(u32 address, F&& func)
        -> void { (*m_write_callbacks)[(address - LowerBound) / 2] = std::forward<F>(func); }
private:
    [[nodiscard]]static auto undefined_read(u32 address) 
        -> u16 { spdlog::warn("Undefined read at address: {:#010x}", address); return 0xebeb;}
    static auto undefined_write(u32 address, u16 data) 
        -> void { spdlog::warn("Undefined write of {1:#x} to address: {0:#010x}", address, data); }
private:
    std::unique_ptr<std::array<std::function<u16(void)>, memsize/2>> m_read_callbacks;
    std::unique_ptr<std::array<std::function<void(u16)>, memsize/2>> m_write_callbacks;
};
}
#endif