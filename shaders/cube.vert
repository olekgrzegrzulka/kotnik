#version 450 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in uint pack;
layout (location = 2) in uint foliage_;

uniform mat4 camera_matrix;
uniform vec3 light_dir;
uniform vec3 camera_pos;
uniform float alpha;
uniform float fog_start = 180.0;
uniform float fog_end = 240.0;

out vec2 uv;
out float brightness;
out float fog_factor;
out float alpha_frag;
out vec3 foliage;

float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
const float screenWidth = 100.0;
const float PI = 3.141;
float noise(vec2 p, float freq ){
	float unit = screenWidth/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

void main() {
    uv.x = float((pack & 0xFF)) / 16.0 / 2.0;
    uv.y = float((pack & 0xFF00) >> 8) /  16.0 / 2.0;

    vec3 normal;
    normal.x = float((pack & 0x30000) >> 16) - 1.0;
    normal.y = float((pack & 0xc0000) >> 18) - 1.0;
    normal.z = float((pack & 0x300000) >> 20) - 1.0;

    gl_Position = camera_matrix * (vec4(vertex - camera_pos, 1.0));

    foliage.r = float((foliage_ & 0x000000FF) >> 0) / 255.0;
    foliage.g = float((foliage_ & 0x0000FF00) >> 8) / 255.0;
    foliage.b = float((foliage_ & 0x00FF0000) >> 16) / 255.0;
    foliage.rgb += ((noise(vertex.xz, 50.0) - 0.5) * 2.0) * 0.04;

    uint brightness_vertex = ((pack & 0xFF000000) >> 24); // from 0 to 255
    // brightness_vertex = 255;

    float dot = (dot(normal, light_dir) + 1.0) * 0.5;
    brightness = 0.5 + dot * 0.5;
    brightness = min(brightness, float(brightness_vertex) / 255.0);


    float dist = distance(vertex, camera_pos);
    dist = sqrt(
        (vertex.x - camera_pos.x) * (vertex.x - camera_pos.x) +
        (vertex.y - camera_pos.y) * (vertex.y - camera_pos.y) +
        (vertex.z - camera_pos.z) * (vertex.z - camera_pos.z));
    fog_factor = (clamp(dist, fog_start, fog_end) - fog_start) / (fog_end - fog_start);

    alpha_frag = alpha;

    
    
}
