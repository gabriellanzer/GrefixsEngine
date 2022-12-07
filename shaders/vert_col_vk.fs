#version 450
#define VULKAN 100

layout(location = 0) out vec4 color;

// layout(binding = 0, set=0) uniform FUBO {
//     mat4 viewProj;
//     uniform float time;
//     uniform float perlin;
// } frameUBO;

// void main(){
//     vec3 rgb = vec3(1.0f, 0.7f, 0.5f) * clamp(frameUBO.perlin, 0.0f, 1.0f) * 0.8f + 0.2f;
//     color = vec4(rgb, 1.0f);
// }


void main() {
    color = vec4(1.0f, 0.7f, 0.5f, 1.0f);
}