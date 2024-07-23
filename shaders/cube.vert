#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in uint pack;

uniform mat4 camera_matrix;
uniform vec3 light;
uniform vec3 camera_position;
uniform float _alpha;

out vec2 uv;
out float brightness;
out float fog_factor;
out float alpha;

void main() {
    uv.x = float((pack & 0xFF)) / 16.0 / 2.0;
    uv.y = float((pack & 0xFF00) >> 8) /  16.0 / 2.0;

    vec3 normal;
    normal.x = float((pack & 0x30000) >> 16) - 1.0;
    normal.y = float((pack & 0xc0000) >> 18) - 1.0;
    normal.z = float((pack & 0x300000) >> 20) - 1.0;

    uint brightness_vertex = ((pack & 0xFF000000) >> 24); // from 0 to 255
    // brightness_vertex = 255;


    float dot = (dot(normal, light) + 1.0) * 0.5;
    brightness = 0.5 + dot * 0.5;
    brightness = min(brightness, float(brightness_vertex) / 255.0);

    fog_factor = clamp(
        (distance(vertex, camera_position) - 150) * 0.005,
        0.0, 1.0
    );

    alpha = _alpha;

    gl_Position = camera_matrix * (vec4(vertex - camera_position, 1.0));
    
}
