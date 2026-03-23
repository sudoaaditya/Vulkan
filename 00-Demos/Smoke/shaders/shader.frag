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

float sampleNoise(vec2 uv) {
    return texture(uTextureSampler, uv).r;
}

float fbmSmoke(vec2 uv) {
    float sum = 0.0;
    float amp = 0.55;
    mat2 rot = mat2(1.6, 1.2, -1.2, 1.6);

    for (int i = 0; i < 4; ++i) {
        sum += amp * sampleNoise(uv);
        uv = rot * uv * 0.55;
        amp *= 0.55;
    }

    return sum;
}

void main(void) {
    float t = uMVP.time;

    vec2 smokeUv = outTexCoord * vec2(0.55, 0.35);
    smokeUv.y -= t * 0.10;
    smokeUv.x += sin(outTexCoord.y * 4.5 + t * 0.4) * 0.06;

    vec2 detailUv = outTexCoord * vec2(1.4, 0.75) + vec2(0.0, -t * 0.065);
    float baseLayer = fbmSmoke(smokeUv);
    float detailLayer = fbmSmoke(detailUv + vec2(baseLayer * 0.22, 0.0));

    float smoke = mix(baseLayer, detailLayer, 0.42);
    smoke = smoothstep(0.26, 0.80, smoke);

    smoke *= smoothstep(0.0, 0.11, outTexCoord.x);
    smoke *= smoothstep(1.0, 0.87, outTexCoord.x);
    smoke *= smoothstep(0.0, 0.04, outTexCoord.y);
    smoke *= smoothstep(1.0, 0.32, outTexCoord.y);

    float centerFade = 1.0 - smoothstep(0.25, 0.62, abs(outTexCoord.x - 0.5));
    smoke *= mix(0.72, 1.0, centerFade);

    vec3 coolTint = vec3(0.78, 0.82, 0.90);
    vec3 warmTint = vec3(0.95, 0.93, 0.88);
    vec3 smokeColor = mix(coolTint, warmTint, smoothstep(0.0, 0.35, outTexCoord.y));
    smokeColor = mix(smokeColor * 0.75, smokeColor * 1.1, smoke);

    // Premultiplied alpha produces softer compositing at the plume edges.
    FragColor = vec4(smokeColor * smoke, smoke);
}