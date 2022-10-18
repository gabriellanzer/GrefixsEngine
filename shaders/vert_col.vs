#version 440 core

layout(location = 0) in vec3 vertex;

layout(location = 0) uniform mat4 viewProj;
layout(location = 1) uniform mat4 model;
layout(location = 2) uniform float time;

void main(){
    mat4 mvp = viewProj * model;
    gl_Position = mvp * vec4(vertex, 1.0f);
}