#ifndef FGBA_BUS_HPP
#define FGBA_BUS_HPP

#include <memory>
#include <optional>

#include "emulator/cpudefines.hpp"
#include "fgba-defines.hpp"
namespace fgba::cpu {

class connector {
    using access_read_sig  = auto(void*, address, data_size, word&) -> void;
    using access_write_sig = auto(void*, address, data_size, word)  -> void;
public:
    
    connector() noexcept = default;
    template<typename Mmu>
        requires (not std::same_as<connector, Mmu>)
    connector(Mmu& mmu_impl) noexcept;

    connector(connector const&) noexcept = default;
    auto operator=(connector const&) noexcept 
        -> connector& = default;
        
    auto access_read(address, data_size, word& data_bus)
        -> void;
    auto access_write(address, data_size, word data_bus)
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
    auto load_on(word data) noexcept
        -> void;
    [[nodiscard]]
    auto load_from() const noexcept
        -> word;
    auto access_read(address, data_size)
        -> void;
    auto access_write(address, data_size)
        -> void;
private:
    connector m_connection{};
    word m_data{unitialized_word};
};

template<typename Mmu>
    requires (not std::same_as<connector, Mmu>)
connector::connector(Mmu& mmu_impl) noexcept 
    :
        m_mmu{std::addressof(mmu_impl)},
        m_read_impl{
            [](void* mem, address address, data_size mas, word& data_bus) {
                return static_cast<Mmu*>(mem)->memory_access_read(address, mas, data_bus);
            }
        },
        m_write_impl{
            [](void* mem, address address, data_size mas, word data_bus) {
                return static_cast<Mmu*>(mem)->memory_access_write(address, mas, data_bus);
            }
        } {}
} // namespace fgab::cpu

#endif
