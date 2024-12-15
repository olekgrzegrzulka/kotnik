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

    // vec4 fog_color = vec4(225.0 / 255.0, 215.0 / 255.0, 180.0 / 255.0, 1.0);
    // fog_color.rgb *= 1.02;
    // color.rgb = mix(color.rgb, fog_color.rgb, fog_factor).rgb;

    FragColor = color;
}
