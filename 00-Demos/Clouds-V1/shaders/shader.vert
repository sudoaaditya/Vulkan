#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out float outViewPosZ;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} uMVP;

void main (void) {

    // code
    vec4 worldPos = uMVP.modelMatrix * vPosition;
    vec4 viewPos = uMVP.viewMatrix * worldPos;

    gl_Position = uMVP.projectionMatrix * viewPos;

    outTexCoord = vTexCoord;
    outViewPosZ = -viewPos.z;
}