#version 450 core

layout (binding = 0) uniform sampler2D atlas;
layout (binding = 1) uniform sampler2D atlas_foliage_mask;

out vec4 FragColor;

in float brightness;
in vec2 uv;
in float fog_factor;
in float alpha_frag;
in vec3 foliage;

void main() {
    vec4 color = texture(atlas, uv).rgba;
    float is_foliage = texture(atlas_foliage_mask, uv).a;
    color.rgb = mix(color.rgb, clamp(foliage * dot(color.rgb, vec3(0.299, 0.587, 0.114)), 0.0, 1.0), is_foliage);
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
