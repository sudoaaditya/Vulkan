#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    // for sea
    float time;
    float bigWavesElevation;
    vec2 bigWavesFrequency;
    float bigWavesSpeed;

    float smallWavesElevation;
    float smallWavesFrequency;
    float smallWavesSpeed;
    float smallWavesIteration;

    vec3 depthColor;
    vec3 surfaceColor;
    float colorOffset;
    float colorMultiplier;
} uMVP;

layout(location = 0) in float elevationOut;

layout(location = 0) out vec4 FragColor;

void main(void) {

    float mixedStrength = (elevationOut + uMVP.colorOffset) * uMVP.colorMultiplier;
    vec3 mixedColor = mix(uMVP.depthColor, uMVP.surfaceColor, mixedStrength);
    FragColor = vec4(mixedColor, 1.0);
}
