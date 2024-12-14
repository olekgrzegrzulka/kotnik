#version 450 core

uniform vec3 color_top;
uniform vec3 color_up;
uniform vec3 color_mid;
uniform vec3 color_down;
uniform vec3 color_bottom;

in float u;

out vec4 FragColor;

void main() {
    vec3 color;

    if (u < 0.0) {
        color.r = mix(color_bottom.r, color_down.r, (u + 1.0) / 1.0);
        color.g = mix(color_bottom.g, color_down.g, (u + 1.0) / 1.0);
        color.b = mix(color_bottom.b, color_down.b, (u + 1.0) / 1.0);
    } else if (u >= 0.0 && u < 0.3) {
        color.r = mix(color_down.r, color_mid.r, (u + 0.0) / 0.3);
        color.g = mix(color_down.g, color_mid.g, (u + 0.0) / 0.3);
        color.b = mix(color_down.b, color_mid.b, (u + 0.0) / 0.3);
    } else if (u >= 0.3 && u < 0.7) {
        color.r = mix(color_mid.r, color_up.r, (u - 0.3) / 0.4);
        color.g = mix(color_mid.g, color_up.g, (u - 0.3) / 0.4);
        color.b = mix(color_mid.b, color_up.b, (u - 0.3) / 0.4);
    } else if (u >= 0.7) {
        color.r = mix(color_up.r, color_top.r, (u - 0.7) / 0.3);
        color.g = mix(color_up.g, color_top.g, (u - 0.7) / 0.3);
        color.b = mix(color_up.b, color_top.b, (u - 0.7) / 0.3);
    } else {
        color = vec3(1.0, 0.0, 0.0);
    }

    FragColor = vec4(color, 1.0);
}
