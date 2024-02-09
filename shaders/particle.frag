#version 450 core

layout (binding = 0) uniform sampler2D texture;

out vec4 FragColor;

in float brightness;
in vec2 uv;

void main() {
    vec3 color = texture(atlas, uv).rgb;
    FragColor = vec4(color, 1.0) * brightness;
}
