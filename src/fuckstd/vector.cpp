module;
#include <vector>
#include <concepts>
export module fstd.vector;

import fstd.ilist;
import fstd.exception;

namespace fstd {
export template<typename T>
class vector : private std::vector<T> {
    using underlying = std::vector<T>;
    using val_type = underlying::value_type;
    using ref_type = underlying::reference;
    using cref_type = underlying::const_reference;
    using size_type = underlying::size_type;
public:
    constexpr vector() = default;
    constexpr vector(vector const&) = default;
    constexpr auto operator=(vector const&) 
        -> vector& = default;
    constexpr auto operator=(vector&&) 
        -> vector& = default;
    constexpr vector(vector&&) = default;
    constexpr ~vector() = default;

    constexpr explicit vector(ilist<val_type> list)
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
    using underlying::insert;
    using underlying::resize;
    using underlying::erase;
    using underlying::reserve;
    using underlying::capacity;
    using underlying::emplace;
    using underlying::clear;
    using underlying::swap;
    //change for the checked ones
    using underlying::front;
    using underlying::back;

    [[nodiscard]]constexpr auto operator[](size_type index)
        -> ref_type {
        if (underlying::size() <= index) {
            throw runtime_error{"out of bounds access"};
        }
        return underlying::operator[](index);
    }
    [[nodiscard]]constexpr auto operator[](size_type index) const
        -> cref_type {
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
        -> size_type {
        return static_cast<size_type>(underlying::size());
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
}