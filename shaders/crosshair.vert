#version 450 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 vertex_uv;

uniform float aspect_ratio;

out vec2 uv;

void main() {
    uv = vertex_uv;

    // brightness = light;

    gl_Position =  vec4(vec2(vertex.x / aspect_ratio, vertex.y) / 48, 0.0, 1.0);
}
