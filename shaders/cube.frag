#version 450 core

layout (binding = 0) uniform sampler2D atlas;

out vec4 FragColor;

in float brightness;
in vec2 uv;

void main() {
    vec4 color = texture(atlas, uv).rgba;
    if (color.a < 0.6) {discard;}
    FragColor = vec4(color.rgb, 1.0) * brightness;
}
