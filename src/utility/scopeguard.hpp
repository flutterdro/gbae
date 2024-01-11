#ifndef SCOPE_GUARD_HPP_
#define SCOPE_GUARD_HPP_

#include <exception>
#include <utility>
namespace fgba {
template<typename EF>
class scope_fail {
public:
    scope_fail(EF&& handler) 
        : m_exit_func{std::forward<EF>(handler)} {}
    ~scope_fail() {
        if (std::uncaught_exception()) {
            std::invoke(m_exit_func);
        }
    }
private:
    EF m_exit_func;
};

template<typename EF>
class scope_exit {
public:
    scope_exit(EF&& handler) 
        : m_exit_func{std::forward<EF>(handler)} {}
    ~scope_exit() {
        std::invoke(m_exit_func);
    }
private:
    EF m_exit_func;
};
}
#endif