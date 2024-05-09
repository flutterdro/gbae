#include "emulator/cpu/bus.hpp"
#include "emulator/cpudefines.hpp"
#include <functional>

namespace fgba::cpu {

bus::bus(connector connector) noexcept
    : m_connection{connector}, m_data{unitialized_word} {}

auto bus::connect(connector connector) noexcept 
    -> void { m_connection = connector; }

auto bus::load_on(u32 data) noexcept
    -> void { m_data = data; }

auto bus::load_from() const noexcept
    -> u32 { return m_data; }

auto bus::access_read(address address, data_size mas) 
    -> void {
    m_connection.access_read(address, mas, m_data);
}
auto bus::access_write(address address, data_size mas)
    -> void {
    m_connection.access_write(address, mas, m_data);
}

auto connector::access_read(address address, data_size mas, u32& data_bus)
    -> void {
    std::invoke(m_read_impl, m_mmu, address, mas, data_bus);
}
auto connector::access_write(address address, data_size mas, u32 data_bus)
    -> void {
    std::invoke(m_write_impl, m_mmu, address, mas, data_bus);
}


}
