#version 450 core

layout (binding = 0) uniform sampler2D atlas;

out vec4 FragColor;

in float brightness;
in vec2 uv;
in float fog_factor;
in float alpha;

void main() {
    vec4 color = texture(atlas, uv).rgba;
    if (color.a  < 0.5) {discard;}
    color.rgb *= brightness;
    color.a = alpha;

    vec4 fog_color = vec4(0.68, 0.88, 0.97, 1.0);
    color.rgb = mix(color.rgb, fog_color.rgb, fog_factor).rgb;

    FragColor = color;
}
