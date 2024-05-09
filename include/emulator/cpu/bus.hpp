#ifndef FGBA_BUS_HPP
#define FGBA_BUS_HPP

#include <memory>
#include <optional>

#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
namespace fgba::cpu {

class connector {
    using access_read_sig  = auto(void*, address, data_size, u32&) -> void;
    using access_write_sig = auto(void*, address, data_size, u32)  -> void;
    using read_byte_sig   = auto(void*, address)      -> std::optional<u8>;
    using read_hword_sig  = auto(void*, address)      -> std::optional<u16>;
    using read_word_sig   = auto(void*, address)      -> std::optional<u32>;
    using write_byte_sig  = auto(void*, address, u32) -> void;
    using write_hword_sig = auto(void*, address, u32) -> void;
    using write_word_sig  = auto(void*, address, u32) -> void;
public:
    
    connector() noexcept = default;
    template<typename Mmu>
        requires (not std::same_as<connector, Mmu>)
    connector(Mmu& mmu_impl) noexcept;

    connector(connector const&) noexcept = default;
    auto operator=(connector const&) noexcept 
        -> connector& = default;
        
    auto access_read(address, data_size, u32& data_bus)
        -> void;
    auto access_write(address, data_size, u32 data_bus)
        -> void;
private:
    void* m_mmu{};
    access_read_sig*  m_read_impl{};
    access_write_sig* m_write_impl{};
};

class bus {
public:
    bus() noexcept = default;
    bus(connector) noexcept;
    auto connect(connector) noexcept
        -> void;
    auto load_on(u32 data) noexcept
        -> void;
    [[nodiscard]]
    auto load_from() const noexcept
        -> u32;
    auto access_read(address, data_size)
        -> void;
    auto access_write(address, data_size)
        -> void;
private:
    connector m_connection{};
    u32 m_data{unitialized_word};
};

template<typename Mmu>
    requires (not std::same_as<connector, Mmu>)
connector::connector(Mmu& mmu_impl) noexcept 
    :
        m_mmu{std::addressof(mmu_impl)},
        m_read_impl{
            [](void* mem, address address, data_size mas, u32& data_bus) {
                return static_cast<Mmu*>(mem)->memory_access_read(address, mas, data_bus);
            }
        },
        m_write_impl{
            [](void* mem, address address, data_size mas, u32 data_bus) {
                return static_cast<Mmu*>(mem)->memory_access_write(address, mas, data_bus);
            }
        } {}
} // namespace fgab::cpu

#endif
