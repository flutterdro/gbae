#version 330 core
layout (location = 0) in vec2 a_coord;

layout (std140) uniform matrices {
    mat4 projection;
};

uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(a_coord, 1.0, 1.0);
}