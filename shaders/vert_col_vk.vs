#version 450
#define VULKAN 100

layout(location = 0) in vec2 inPosition;

// layout(binding = 0, set=0) uniform FUBO {
//     mat4 viewProj;
//     uniform float time;
//     uniform float perlin;
// } frameUBO;

// layout(binding = 1, set=1) uniform MUBO { 
//     mat4 model;
// } modelUBO;

// void main(){
//     mat4 mvp = frameUBO.viewProj * modelUBO.model;
//     gl_Position = mvp * vec4(vertex, 1.0f);
// }

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
}