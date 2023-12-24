module;
#include <utility>
#include <iterator>
#include <cstddef>
#include <span>
export module fstd.span;

import fstd.ilist;
import fstd.exception;

namespace fstd {
export template<typename T>
class span : private std::span<T> {
private:
    using underlying = std::span<T>;
    using val_t = underlying::value_type;
    using ref_t = underlying::reference;
    using size_type = underlying::size_type;
public:
    constexpr span() noexcept = default;
    constexpr span(span const&) noexcept = default;
    constexpr span(span&&) noexcept = default;
    constexpr auto operator=(span const&) noexcept
        -> span& = default;
    constexpr auto operator=(span&&) noexcept
        -> span& = default;
    constexpr ~span() = default;

    constexpr span(ilist<val_t> list) 
        : underlying(list.m_list) {}
    template<typename... Ts>
        requires std::constructible_from<underlying, Ts...>
    constexpr span(Ts&&... args)
        : underlying(std::forward<Ts>(args)...) {}

    using underlying::begin;
    // using underlying::cbegin;
    using underlying::rbegin;
    // using underlying::crbegin;
    using underlying::end;
    using underlying::rend;
    using underlying::size;
    using underlying::size_bytes;
    using underlying::data;
    template<typename U>
    friend auto as_bytes(fstd::span<U>) noexcept
        -> span<std::byte const>;

    [[nodiscard]]constexpr auto is_empty() const noexcept
        -> bool {
        return underlying::empty;
    }
    [[nodiscard]]constexpr auto front() const
        -> ref_t {
        if (is_empty()) {
            throw runtime_error("accesing empty span");
        }
        return underlying::front();
    }
    [[nodiscard]]constexpr auto back() const
        -> ref_t {
        if (is_empty()) {
            throw runtime_error("accesing empty span");
        }
        return underlying::back();
    }
    [[nodiscard]]constexpr auto operator[](size_type index) const
        -> ref_t {
        if (index >= underlying::size()) {
            throw runtime_error("out of bounds access to span");
        }
        return underlying::operator[](index);
    }
    [[nodiscard]]constexpr auto subspan(size_type offset, size_type count = std::dynamic_extent) const
        -> span<T> {
        if (offset > size() or ((count != std::dynamic_extent) and (count > size() - offset))) {
            throw runtime_error("you bit off more than you could chew");
        }
        return underlying::subspan(offset, count);
    }
    [[nodiscard]]constexpr auto first(size_type count) const
        -> span<T> {
        if (count > size()) {
            throw runtime_error("you bit off more than you could chew");
        }
        return underlying::first(count);
    }
    [[nodiscard]]constexpr auto last(size_type count) const
        -> span<T> {
        if (count > size()) {
            throw runtime_error("you bit off more than you could chew");
        }
        return underlying::last(count);
    }
};
}

namespace fstd {
export template<typename T>
auto as_bytes(fstd::span<T> s) noexcept
    -> span<std::byte const> {
        return std::as_bytes(s.underlying);
    }
}

template<typename T>
inline constexpr bool std::ranges::enable_borrowed_range<fstd::span<T>> = true;

template<typename T>
inline constexpr bool std::ranges::enable_view<fstd::span<T>> = true;