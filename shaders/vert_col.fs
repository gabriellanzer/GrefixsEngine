#version 440 core

layout(location = 0) out vec4 color;

layout(location = 3) uniform float perlin;

void main(){
    vec3 rgb = vec3(1.0f, 0.7f, 0.5f) * clamp(perlin, 0.0f, 1.0f) * 0.8f + 0.2f;
    color = vec4(rgb, 1.0f);
}