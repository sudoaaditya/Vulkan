#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec2 outTexCoord;
layout(binding = 0) uniform Unifroms {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    float cloudOpacity;
    float elapsedTime;
} uMVP;

void main (void) {

    // code
    // move clouds over time in circular motion
    float radius = 5.0;
    float speed = 0.1;
    float angle = uMVP.elapsedTime * 0.5 * speed + uMVP.cloudOpacity; // speed based on opacity
    vec3 offset = vec3(radius * cos(angle), 0.0, radius * sin(angle));
    vec4 animatedPosition = vPosition + vec4(offset, 0.0);
    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uMVP.modelMatrix * animatedPosition;

    outTexCoord = vTexCoord;
}
