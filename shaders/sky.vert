#version 450 core

layout (location = 0) in vec3 vertex;

uniform mat4 camera_matrix;

out float u;

void main() {
    u = vertex.y;
    gl_Position = camera_matrix * vec4(vertex, 1.0);
}
