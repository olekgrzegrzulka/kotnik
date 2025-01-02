#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in uint pack;

uniform mat4 camera_matrix;
uniform vec3 light_dir;
uniform vec3 camera_pos;
uniform float alpha;

out vec2 uv;
out float brightness;
out float fog_factor;
out float alpha_frag;

void main() {
    uv.x = float((pack & 0xFF)) / 16.0 / 2.0;
    uv.y = float((pack & 0xFF00) >> 8) /  16.0 / 2.0;

    vec3 normal;
    normal.x = float((pack & 0x30000) >> 16) - 1.0;
    normal.y = float((pack & 0xc0000) >> 18) - 1.0;
    normal.z = float((pack & 0x300000) >> 20) - 1.0;

    uint brightness_vertex = ((pack & 0xFF000000) >> 24); // from 0 to 255
    // brightness_vertex = 255;


    float dot = (dot(normal, light_dir) + 1.0) * 0.5;
    brightness = 0.5 + dot * 0.5;
    brightness = min(brightness, float(brightness_vertex) / 255.0);

    const float fog_start = 180.0;
    const float fog_end = 240.0;
    float dist = distance(vertex, camera_pos);
    dist = sqrt(
        (vertex.x - camera_pos.x) * (vertex.x - camera_pos.x) +
        (vertex.y - camera_pos.y) * (vertex.y - camera_pos.y) +
        (vertex.z - camera_pos.z) * (vertex.z - camera_pos.z));
    fog_factor = (clamp(dist, fog_start, fog_end) - fog_start) / (fog_end - fog_start);

    alpha_frag = alpha;

    gl_Position = camera_matrix * (vec4(vertex - camera_pos, 1.0));
    
}
