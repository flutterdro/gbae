#ifndef SHADER_HPP_
#define SHADER_HPP_

#include <filesystem>
#include <glad/gl.h>
#include <memory>
#include <string>
#include <span>

#include <glm/glm.hpp>

namespace fgba::gui {
class shader {
public:
    struct source {
        char const* code;
        GLenum type;
    };

    shader(source const&);
    [[nodiscard]]auto id() const noexcept
        -> GLuint;
    auto cleanup() noexcept
        -> void;
private:
    GLuint m_id;
};

class shader_program {
public:
    shader_program() = delete;
    shader_program(std::filesystem::path const&);
    shader_program(std::span<shader>);
    shader_program(auto const&) = delete;

    auto use() const noexcept
        -> void;

    void set(std::string const& name, int value) const noexcept;
    void set(std::string const& name, float value) const noexcept;
    void set(std::string const& name, float value_1, float value_2) const noexcept;
    void set(std::string const& name, float value_1, float value_2, float value_3) const noexcept;
    void set(std::string const& name, glm::vec2 value) const noexcept;
    void set(std::string const& name, glm::vec3 value) const noexcept;
    void set(std::string const& name, glm::mat3 value) const noexcept;
    void set(std::string const& name, glm::mat4 value) const noexcept;
    [[nodiscard]]auto id() const noexcept
        -> GLuint;

    static auto set_projection(glm::mat4 const&) noexcept
        -> void;
    auto bind_block_to(std::string const&, GLuint) const noexcept
        -> void;
    auto cleanup() noexcept
        -> void;
private:
    GLuint m_id;
};
}

#endif