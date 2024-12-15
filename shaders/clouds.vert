#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv_;

uniform mat4 camera_matrix;

out vec2 uv;

void main() {
    uv = uv_;
    gl_Position = camera_matrix * vec4(vertex, 1.0);
}
