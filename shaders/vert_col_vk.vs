#version 450
#define VULKAN 100

// layout(location = 0) in vec3 vertex;
layout(location = 0) out vec3 fragColor;

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

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}