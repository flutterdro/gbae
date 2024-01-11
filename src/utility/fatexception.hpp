#ifndef FAT_EXCEPTION_HPP
#define FAT_EXCEPTION_HPP
#include "fmt/core.h"
#include <__concepts/same_as.h>
#include <__format/concepts.h>
#include <exception>
#include <source_location>
#include <string>
#ifdef __has_include
#   if __has_include(<stacktrace>)
#       include <stacktrace>
#   endif
#endif
#if __cpp_lib_stacktrace
    constexpr bool has_backtrace = true;
#else
    constexpr bool has_backtrace = false;
#endif
namespace fgba {
template<typename DataT>
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

template<>
class fat_exception<void> {
public:
    fat_exception(
        std::string message, 
        std::source_location source = std::source_location::current()
    )
        : m_err_str{std::move(message)}, 
        m_source{source} {}

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
    [[nodiscard]]auto when() const noexcept
        -> int {
        return 0;
    }
    std::string m_err_str;
    std::source_location const m_source;
};
using runtime_error = fat_exception<void>;
}
template <>
struct fmt::formatter<std::source_location> {
    constexpr auto parse(format_parse_context& ctx) 
        -> format_parse_context::iterator {
        return ctx.begin();
    }
    constexpr auto format(std::source_location const& loc, format_context& ctx) const 
        -> format_context::iterator {
        return fmt::format_to(ctx.out(), "{} in the file {}{}:{}",
            loc.function_name(),
            loc.file_name(),
            loc.line(),
            loc.column()
        );
    }
};

template<typename DataT>
struct fmt::formatter<fgba::fat_exception<DataT>> {
    constexpr auto parse(format_parse_context& ctx) 
        -> format_parse_context::iterator {
        return ctx.begin();
    }
    constexpr auto format(fgba::fat_exception<DataT> const& e, format_context& ctx) const 
        -> format_context::iterator {
        fmt::format_to(ctx.out(), 
            "exception was thrown from:\n{1}\n"
            "{0}\n",
            e.what(),
            e.where()
        );     
        if constexpr (std::formattable<DataT, char> and not std::same_as<DataT, void>) {
            fmt::format_to(ctx.out(), "embeded data:\n{}\n", e.data());
        } else if constexpr (not std::same_as<DataT, void>) {
            fmt::format_to(ctx.out(), "embeded data:\n<formatter not defined>\n");
        }
        if constexpr (has_backtrace) {
            fmt::format_to(ctx.out(), "printing backtrace:\n{}", e.when());
        } else {
            fmt::format_to(ctx.out(), "printing backtrace:\n<backtrace isn't supported>");
        }

        return ctx.out();
    }
};

#endif