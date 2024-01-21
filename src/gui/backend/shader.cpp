#include "shader.hpp"
#include "fatexception.hpp"
#include "fmt/core.h"
#include "glad/gl.h"
#include "scopeguard.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <fstream>
#include <source_location>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>

namespace {
[[nodiscard]]auto load_src_from_file(std::filesystem::path const& path) 
    -> std::string {
    std::ifstream fin{path};
    std::stringstream sstream;
    sstream << fin.rdbuf();
    return sstream.str();
}
}

namespace fgba::gui {

void shader_program::set(std::string const& name, int value) const noexcept {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}
void shader_program::set(std::string const& name, float value) const noexcept {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}
void shader_program::set(std::string const& name, float value_1,
                 float value_2) const noexcept {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), value_1, value_2);
}
void shader_program::set(std::string const& name, glm::vec2 value) const noexcept {
    set(name, value.x, value.y);
}
void shader_program::set(std::string const& name, float value_1, float value_2,
                 float value_3) const noexcept {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), value_1, value_2,
                value_3);
}
void shader_program::set(std::string const& name, glm::vec3 value) const noexcept {
    set(name, value.x, value.y, value.z);
}
void shader_program::set(std::string const& name, glm::mat3 value) const noexcept {
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE,
                       glm::value_ptr(value));
}
void shader_program::set(std::string const& name, glm::mat4 value) const noexcept {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE,
                       glm::value_ptr(value));
}

auto shader_program::bind_block_to(std::string const& block, GLuint index) const noexcept
    -> void {
    glUniformBlockBinding(
        m_id, 
        glGetUniformBlockIndex(m_id, block.c_str()), 
        index
    );
}

shader::shader(source const& source) {
    m_id = glCreateShader(source.type);
    glShaderSource(m_id, 1, &source.code, nullptr);
    glCompileShader(m_id);
    int success;
    char info[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (not success) {
        glGetShaderInfoLog(m_id, 512, nullptr, info);
        throw fgba::runtime_error{info, std::source_location::current()};
    }
}
auto shader::id() const noexcept
    -> GLuint {
    return m_id;
}
auto shader_program::id() const noexcept
    -> GLuint {
    return m_id;
}
auto shader::cleanup() noexcept
    -> void {
    glDeleteShader(m_id);
}

shader_program::shader_program(std::filesystem::path const& shader_dir) {
    std::vector<shader> shaders{};
    scope_exit guard{
        [&shaders] {
            for (auto&& shader : shaders) {
                shader.cleanup();
            }
        }
    };
    auto is_shader_sourcefile = [](std::filesystem::directory_entry const& file) {
        if (file.is_regular_file()) {
            auto const extension = file.path().extension().string();
            return  (extension == ".gs") or (extension == ".vs") or (extension == ".fs");
        } else {
            return false;
        }
    };
    auto&& shader_entries = std::filesystem::recursive_directory_iterator(shader_dir)
        |std::views::filter(is_shader_sourcefile);
    for (auto&& entry : shader_entries) {
        GLenum shader_type;
        auto const extension = entry.path().extension().string();
        if (extension == ".vs") {
            shader_type = GL_VERTEX_SHADER;
        } else if (extension == ".fs") {
            shader_type = GL_FRAGMENT_SHADER;
        } else if (extension == ".gs") {
            shader_type = GL_GEOMETRY_SHADER;
        } else throw fgba::runtime_error("invalid extension");
        std::string src = load_src_from_file(entry.path());
        shaders.emplace_back(shader::source{.code = src.c_str(), .type = shader_type});
    }
    *this = shader_program{std::span{shaders}};
}
shader_program::shader_program(std::span<shader> shaders) {
    m_id = glCreateProgram();
    for (auto&& shader : shaders) {
        assert(glIsShader(shader.id()));
        fmt::println("{}", shader.id());
        glAttachShader(m_id, shader.id());
    }
    glLinkProgram(m_id);
    int success;
    char info[512];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (not success) {
        glGetProgramInfoLog(m_id, 512, nullptr, info);
        throw fgba::runtime_error{info, std::source_location::current()};
    }
}

auto shader_program::use() const noexcept
    -> void {
    glUseProgram(m_id);
}

auto shader_program::cleanup() noexcept
    -> void {
    glDeleteProgram(m_id);
}
}