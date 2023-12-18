module;

#include <exception>
#include <source_location>
#include <string>

export module utility;

export class traced_exception : std::exception {
public:
    traced_exception(std::string message, std::source_location source = std::source_location::current()) 
        : m_message(std::move(message)), m_source(source) {}
    auto what() const noexcept
        -> char const* override {
        return m_message.c_str();
    }
    auto source() const noexcept
        -> std::source_location const& {
        return m_source;
    }
    std::source_location m_source;
    std::string m_message;
};