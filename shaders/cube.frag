#version 450 core

layout (binding = 0) uniform sampler2D atlas;

out vec4 FragColor;

in float brightness;
in vec2 uv;
in float fog_factor;
in float alpha_frag;

void main() {
    vec4 color = texture(atlas, uv).rgba;
    if (color.a  < 0.5) {discard;}
    color.rgb *= brightness;
    color.a = alpha_frag;

    if (fog_factor > 0.99) {discard;}

    vec4 fog_color = vec4(215, 220, 205, 255.0);
    fog_color /= 255.0;
    fog_color.rgb *= 1.02;
    color.rgb = mix(color.rgb, fog_color.rgb, fog_factor).rgb;

    FragColor = color;
}
