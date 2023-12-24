module;
#include <iterator>
#include <sys/_types/_ssize_t.h>
#include <concepts>
#include <initializer_list>
#include <vector>
#include <type_traits>
#include <utility>
#include <source_location>
#include <string>
#ifdef __has_include
#   if __has_include(<stacktrace>)
#       include <stacktrace>
#   endif
#endif

export module utility;
export template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<typename... Ts>
overloaded(Ts&&...) -> overloaded<std::decay_t<Ts>...>;
namespace fstd {
export template<typename DataT>
class fat_exception {
public:
    fat_exception(
        std::string message, 
        DataT data, 
        std::source_location source = std::source_location::current()
#if __cpp_lib_stacktrace
        std::stacktrace trace = std::stacktrace::current()
#endif
    ) 
        : m_err_str{std::move(message)}, 
        m_user_data{std::move(data)}, 
        m_source{source}
#if __cpp_lib_stacktrace
        ,m_trace{trace}
#endif
        {}

    [[nodiscard]]auto what()
        -> std::string& {
        return m_err_str;
    }
    [[nodiscard]]auto what() const noexcept
        -> std::string const& {
        return m_err_str;
    }

    [[nodiscard]]auto where() const noexcept
        -> std::source_location const& {
        return m_source;
    }
#if __cpp_lib_stacktrace
    [[nodiscard]]auto when() const noexcept
        -> std::stacktrace const& {
        return m_trace;
    }
#else
    [[nodiscard]]auto when() const noexcept
        -> int {
        return 0;
    }
#endif
    [[nodiscard]]auto data() 
        -> DataT& {
        return m_user_data;
    }
    [[nodiscard]]auto data() const noexcept
        -> DataT const& {
        return m_user_data;
    }
    std::string m_err_str;
    DataT m_user_data;
    std::source_location const m_source;
#if __cpp_lib_stacktrace
    std::stacktrace const m_trace;
#endif
};

export template<>
class fat_exception<void> {
public:
    fat_exception(
        std::string message, 
        std::source_location source = std::source_location::current()
#if __cpp_lib_stacktrace
        std::stacktrace trace = std::stacktrace::current()
#endif
    ) 
        : m_err_str{std::move(message)}, 
        m_source{source}
#if __cpp_lib_stacktrace
        ,m_trace{trace}
#endif
        {}

    [[nodiscard]]auto what()
        -> std::string& {
        return m_err_str;
    }
    [[nodiscard]]auto what() const noexcept
        -> std::string const& {
        return m_err_str;
    }

    [[nodiscard]]auto where() const noexcept
        -> std::source_location const& {
        return m_source;
    }
#if __cpp_lib_stacktrace
    [[nodiscard]]auto when() const noexcept
        -> std::stacktrace const& {
        return m_trace;
    }
#else
    [[nodiscard]]auto when() const noexcept
        -> int {
        return 0;
    }
#endif
    std::string m_err_str;
    std::source_location const m_source;
#if __cpp_lib_stacktrace
    std::stacktrace const m_trace;
#endif
};
using runtime_error = fat_exception<void>;
}

export namespace fstd {

template<typename T>
struct ilist {
    constexpr ilist(std::initializer_list<T> list)
        : m_list(list) {}
    std::initializer_list<T> m_list;
};
template<typename T>
class vector : private std::vector<T> {
using underlying = std::vector<T>;
public:
    constexpr vector() = default;
    constexpr vector(vector const&) = default;
    constexpr auto operator=(vector const&) 
        -> vector& = default;
    constexpr auto operator=(vector&&) 
        -> vector& = default;
    constexpr vector(vector&&) = default;
    constexpr ~vector() = default;

    constexpr explicit vector(ilist<T> list)
        : underlying(list.m_list) {}
    template<typename... Ts>
        requires std::constructible_from<underlying,  Ts...>
    constexpr explicit vector(Ts&&... args)
        : underlying(std::forward<Ts>(args)...) {}

    using underlying::push_back;
    using underlying::emplace_back;
    using underlying::begin;
    using underlying::end;
    using underlying::cbegin;
    using underlying::cend;
    using underlying::rbegin;
    using underlying::rend;
    using underlying::crbegin;
    using underlying::crend;
    using underlying::data;
    
    [[nodiscard]]constexpr auto operator[](ssize_t index)
        -> T& {
        if (underlying::size() <= index) {
            throw runtime_error{"out of bounds access"};
        }
        return underlying::operator[](index);
    }
    [[nodiscard]]constexpr auto operator[](ssize_t index) const
        -> T const& {
        if (size() <= index) {
            throw runtime_error{"out of bounds access"};
        }
        return underlying::operator[](index);
    }
    constexpr auto pop_back() 
        -> void {
        if (is_empty()) {
            throw runtime_error{"atempt to pop back empty vector"};
        }
        underlying::pop_back();
    }

    [[nodiscard]]constexpr auto size() const noexcept
        -> ssize_t {
        return static_cast<ssize_t>(underlying::size());
    }
    [[nodiscard]]constexpr auto is_empty() const noexcept
        -> bool {
        return underlying::empty();
    }
};
template<typename T>
vector(ilist<T>) -> vector<T>;
template<std::input_iterator It>
vector(It, It) -> vector<typename std::iterator_traits<It>::value_type>;
template<>
class vector<bool> : private std::vector<char> {
using underlying = std::vector<char>;
public:
    constexpr vector() = default;
    constexpr vector(vector const&) = default;
    constexpr auto operator=(vector const&) 
        -> vector& = default;
    constexpr auto operator=(vector&&) 
        -> vector& = default;
    constexpr vector(vector&&) = default;
    constexpr ~vector() = default;

    constexpr explicit vector(ilist<bool> list) 
        : underlying(list.m_list.begin(), list.m_list.end()) {}
    template<typename... Ts>
        requires std::constructible_from<underlying,  Ts...>
    constexpr explicit vector(Ts&&... args)
        : underlying(std::forward<Ts>(args)...) {}

    using underlying::push_back;
    using underlying::emplace_back;
    using underlying::begin;
    using underlying::end;
    using underlying::cbegin;
    using underlying::cend;
    using underlying::rbegin;
    using underlying::rend;
    using underlying::crbegin;
    using underlying::crend;
    using underlying::data;
    
    [[nodiscard]]constexpr auto operator[](ssize_t index)
        -> char& {
        if (underlying::size() <= index) {
            throw runtime_error{"out of bounds access"};
        }
        return underlying::operator[](index);
    }
    [[nodiscard]]constexpr auto operator[](ssize_t index) const
        -> char const& {
        if (size() <= index) {
            throw runtime_error{"out of bounds access"};
        }
        return underlying::operator[](index);
    }
    constexpr auto pop_back() 
        -> void {
        if (is_empty()) {
            throw runtime_error{"atempt to pop back empty vector"};
        }
        underlying::pop_back();
    }

    [[nodiscard]]constexpr auto size() const noexcept
        -> ssize_t {
        return static_cast<ssize_t>(underlying::size());
    }
    [[nodiscard]]constexpr auto is_empty() const noexcept
        -> bool {
        return underlying::empty();
    }
};
}