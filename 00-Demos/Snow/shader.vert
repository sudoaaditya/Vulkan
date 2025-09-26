#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in float aRandom; // Random attribute

float PI = 3.141592653589793238;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    // for snow
    float radius;
    float size;
    float time;
    int useTexture;
    float snowBound;
} uMVP;


void main(void) {
    vec4 modelPosition = uMVP.modelMatrix * vPosition;
    modelPosition.y = mod(modelPosition.y - uMVP.time * 0.2, uMVP.snowBound);

    modelPosition.z = mod(modelPosition.z + (sin((uMVP.time + modelPosition.x) * 0.5)) *
        uMVP.radius * 0.8, uMVP.snowBound) - (uMVP.snowBound / 2.0);

    modelPosition.x = mod(modelPosition.x +
        (cos((uMVP.time + modelPosition.z) * 0.5)) *
        uMVP.radius, uMVP.snowBound) - (uMVP.snowBound / 2.0);

    vec4 viewPosition = uMVP.viewMatrix * modelPosition;
    vec4 projectionPosition = uMVP.projectionMatrix * viewPosition;

    gl_Position = projectionPosition;
    gl_PointSize = uMVP.size * aRandom * (1.0 / -viewPosition.z);


}