#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 2) in uint pack;
//      uv.x   uv.y    
// 0 x  FF     FF      FFFF

uniform mat4 matrix;
uniform vec3 light;

out vec2 uv;
out float brightness;

void main() {
    uv.x = float((pack & 0xFF)) / 16.0 / 2.0;
    uv.y = float((pack & 0xFF00) >> 8) /  16.0 / 2.0;

    vec3 normal;
    normal.x = ((pack & 0x30000) >> 16) - 1.0;
    normal.y = ((pack & 0xc0000) >> 18) - 1.0;
    normal.z = ((pack & 0x300000) >> 20) - 1.0;

    uint brightness_vertex = ((pack & 0x3FC00000) >> 22); // from 0 to 255


    float dot = (dot(normal, light) + 1.0) * 0.5;
    brightness = 0.5 + dot * 0.5 - (1.0 - float(brightness_vertex) / 255.0);
    // brightness = 1.0;

    gl_Position =  matrix * (vec4(vertex, 1.0));
}
