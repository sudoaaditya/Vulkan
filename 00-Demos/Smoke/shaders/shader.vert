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

void main (void) {

    // code
    vec3 tPosition = vPosition.xyz;
    outTexCoord = vTexCoord;

    float twistPerlin = texture(
        uTextureSampler,
        vec2(0.5, outTexCoord.y * 0.2 - uMVP.time * 0.005)
    ).r;

    float angle = twistPerlin * 10.0;
    tPosition.xz = rotate2D(tPosition.xz, angle);

    // wind
    vec2 windOffset = vec2(
        texture(uTextureSampler, vec2(0.25, uMVP.time * 0.01)).r - 0.5,
        texture(uTextureSampler, vec2(0.75, uMVP.time * 0.01)).r - 0.5
    );
    windOffset *= pow(vTexCoord.y, 2.0) * 10.0;
    tPosition.xz += windOffset;

    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uMVP.modelMatrix * vec4(tPosition, 1.0);

}
