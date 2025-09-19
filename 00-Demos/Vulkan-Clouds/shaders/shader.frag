#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;

layout(binding = 0) uniform Uniforms {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    float cloudOpacity;
    float elapsedTime;
} uniforms;
layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void) {

    vec4 baseColor = texture(uTextureSampler, outTexCoord);

    FragColor = vec4(baseColor.rgb, baseColor.a * uniforms.cloudOpacity);
}