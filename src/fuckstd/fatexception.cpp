module;
#include <source_location>
#include <string>
#ifdef __has_include
#   if __has_include(<stacktrace>)
#       include <stacktrace>
#   endif
#endif
export module fstd.exception;
export namespace fstd {
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