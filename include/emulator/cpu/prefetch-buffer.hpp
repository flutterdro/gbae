#ifndef FGBA_PREFETCH_BUFFER_HPP_AKJSNCWRL
#define FGBA_PREFETCH_BUFFER_HPP_AKJSNCWRL

#include "fgba-defines.hpp"
#include <array>
#include <cstring>

namespace fgba::cpu {
// this just a low-effort fixed size queue implementation
// utilizes hopes and dreams and heavily exploits goodwill of the user
class prefetch_buffer {
public:
    template<typename T>
    [[nodiscard]]auto read() noexcept 
        -> T {
        T ret;

        std::memcpy(&ret, m_buffer.data() + m_begin, sizeof(T));

        m_begin += sizeof(T);
        m_begin %= 8;

        return ret;
    }
    template<typename T>
    auto write(T data) noexcept 
        -> void {
        std::memcpy(m_buffer.data() + m_end, &data, sizeof(T));

        m_end += sizeof(T);
        m_end %= 8;
    }
    auto flush() noexcept 
        -> void { m_begin = 0; m_end = 0; }
private:
    std::array<std::byte, 8> m_buffer{};
    u8 m_begin{};
    u8 m_end{};
};
}

#endif // !FGBA_PREFETCH_BUFFER_HPP_AKJSNCWRL

