#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    
    float time;
} uMVP;

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void) {
    //scale and animate
    vec2 smokeUv = outTexCoord;
    smokeUv.x *= 0.5;
    smokeUv.y *= 0.3;
    smokeUv.y -= uMVP.time * 0.03;

    // Smoke
    float smoke = texture(uTextureSampler, smokeUv).r;

    smoke = smoothstep(0.4, 1.0, smoke);

    smoke *= smoothstep(0.0, 0.1, outTexCoord.x);
    smoke *= smoothstep(1.0, 0.9, outTexCoord.x);
    smoke *= smoothstep(0.0, 0.1, outTexCoord.y);
    smoke *= smoothstep(1.0, 0.4, outTexCoord.y);

    // gl_FragColor = vec4(vUv, 1.0, 1.0);
    // FragColor = vec4(0.6, 0.3, 0.2, smoke);
    // white smoke
    FragColor = vec4(1.0, 1.0, 1.0, smoke);
}