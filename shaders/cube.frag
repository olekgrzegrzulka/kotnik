#version 450 core

layout (binding = 0) uniform sampler2D atlas;

out vec4 FragColor;

in float brightness;
in vec2 uv;

void main() {
    vec3 color = texture(atlas, uv).rgb;
    if (color == vec3(0.0, 0.0, 0.0)) {discard; }
    // FragColor = vec4(color, 1.0);
    FragColor = vec4(color, 1.0) * brightness;
}
