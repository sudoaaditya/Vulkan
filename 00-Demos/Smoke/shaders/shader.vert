#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec2 outTexCoord;
layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    
    float time;
} uMVP;

layout(binding = 1) uniform sampler2D uTextureSampler;

vec2 rotate2D(vec2 value, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat2 m = mat2(c, s, -s, c);
    return m * value;
}

float sampleNoise(vec2 uv) {
    return texture(uTextureSampler, uv).r;
}

void main (void) {

    vec3 tPosition = vPosition.xyz;
    outTexCoord = vTexCoord;
    float t = uMVP.time;
    float verticalMask = smoothstep(0.0, 1.0, outTexCoord.y);

    float twistPerlinA = sampleNoise(vec2(0.18, outTexCoord.y * 0.35 - t * 0.04));
    float twistPerlinB = sampleNoise(vec2(0.82, outTexCoord.y * 0.70 - t * 0.09));
    float angle = ((twistPerlinA * 0.75 + twistPerlinB * 0.25) - 0.5) * 3.2 * verticalMask;
    tPosition.xz = rotate2D(tPosition.xz, angle);

    float windA = sampleNoise(vec2(0.27 + outTexCoord.y * 0.05, t * 0.05));
    float windB = sampleNoise(vec2(0.73 - outTexCoord.y * 0.05, t * 0.04 + 0.35));
    vec2 windOffset = vec2(windA - 0.5, windB - 0.5);
    windOffset *= pow(verticalMask, 1.55) * 2.4;

    float billowNoise = sampleNoise(outTexCoord * vec2(0.8, 1.6) + vec2(t * 0.03, -t * 0.05));
    float billowOffset = (billowNoise - 0.5) * 0.35 * verticalMask;
    tPosition.x += windOffset.x + billowOffset;
    tPosition.z += windOffset.y;

    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uMVP.modelMatrix * vec4(tPosition, 1.0);

}
