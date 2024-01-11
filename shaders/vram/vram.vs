#version 410 core
layout (location = 0) in vec4 a_texcoord;

layout (std140) uniform matrices {
    mat4 projection;
};
uniform mat4 model;
out vec2 texcoord;

void main() {
    texcoord = a_texcoord.zw;
    gl_Position = projection * model * vec4(a_texcoord.xy, 0.0, 0.0);
}