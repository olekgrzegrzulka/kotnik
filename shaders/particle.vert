#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 _uv;

uniform mat4 matrix;
uniform vec3 light;

out vec2 uv;
out float brightness;

void main() {
    uv = _uv;

    brightness = light;

    gl_Position =  matrix * (vec4(vertex, 1.0));
}
