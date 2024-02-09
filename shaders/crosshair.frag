#version 450 core

layout (binding = 0) uniform sampler2D tex;

out vec4 FragColor;

// in float brightness;
in vec2 uv;

void main() {
    vec4 color = texture(tex, uv).rgba;
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    if (color == vec4(0.0, 0.0, 0.0, 0.0)) {discard;}
    FragColor = vec4(color);
}
