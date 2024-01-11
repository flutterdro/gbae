#version 410 core

in vec2 texcoord;
out vec4 frag_color;

uniform sampler2D tex;

void main() {
    frag_color = texture(tex, texcoord);
}