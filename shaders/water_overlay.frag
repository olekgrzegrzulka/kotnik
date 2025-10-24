#version 450 core

uniform vec4 color;
in float u;

out vec4 FragColor;

void main() {
    FragColor = vec4(color);
}
