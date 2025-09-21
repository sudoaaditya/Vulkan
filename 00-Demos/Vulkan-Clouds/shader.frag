#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec4 outAnimatedPosition;


layout(binding = 0) uniform Uniforms {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    float cloudOpacity;
    float elapsedTime;
} uniforms;
layout(binding = 1) uniform sampler2D uTextureSampler[2];

layout(location = 0) out vec4 FragColor;

void main(void) {

    vec4 texOne = texture(uTextureSampler[0], outTexCoord);
    vec4 texTwo = texture(uTextureSampler[1], outTexCoord);

    vec4 baseColor = mix(texOne, texTwo, uniforms.cloudOpacity * uniforms.elapsedTime * 0.5); // Blend based on time

    FragColor = vec4(baseColor.rgb, baseColor.a * uniforms.cloudOpacity);
}