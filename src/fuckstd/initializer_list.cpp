module;
#include <initializer_list>
export module fstd.ilist;

namespace fstd {
export template<typename T>
struct ilist {
    constexpr ilist(std::initializer_list<T> list)
        : m_list(list) {}
    std::initializer_list<T> m_list;
};
}