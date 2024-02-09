#version 450 core

layout (location = 0) in vec3 vertex;

uniform mat4 matrix;
uniform vec3 position;

void main() {
    gl_Position =  matrix * (vec4(position + vertex, 1.0));
}
