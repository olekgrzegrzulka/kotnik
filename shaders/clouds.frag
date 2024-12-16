#version 450 core

layout (binding = 0) uniform sampler2D tex;

uniform int t;
uniform vec3 color;
uniform float scale;
uniform float speed;

in vec2 uv;

out vec4 FragColor;

void main() {
    vec2 uv2 = vec2(uv.x / scale + t * speed * 0.001, uv.y / scale);
    float alpha = min(
        sin(uv.x * 3.142),
        sin(uv.y * 3.142)
    );
    vec4 texture_color = texture(tex, uv2);
    alpha *= texture_color.a;
    FragColor = vec4(color.rgb, alpha);
}
