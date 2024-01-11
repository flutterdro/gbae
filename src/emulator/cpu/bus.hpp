#ifndef FGBA_BUS_HPP
#define FGBA_BUS_HPP

#include <concepts>
#include <functional>

#include "../defines.hpp"
namespace fgba::cpu {

class bus {
public:
struct signals {
    u32 address;
    unsigned nrw:1;
    unsigned mas:2;
    unsigned nopc:1;
    unsigned ntrans:1;
    unsigned lock:1;
    unsigned tbit:1;
};
public:
    template<std::invocable<signals> F>
    auto conect_read(F&& func) 
        -> void {
        m_read_callback = std::forward<F>(func);
    }
    template<std::invocable<u32, signals> F>
    auto conect_write(F&& func)
        -> void {
        m_write_callback = std::forward<F>(func);
    }
    auto read(signals signals) const
        -> u32 {
        return m_read_callback(signals);
    }
    auto write(u32 data, signals signals)
        -> void {
        m_write_callback(data, signals);
    }
private:
    std::function<u32(signals)> m_read_callback;
    std::function<void(u32, signals)> m_write_callback;
};
}

#endif