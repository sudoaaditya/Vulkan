#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

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

void main() {
    vec2 uv = gl_PointCoord;
    float distanceToCenter = distance(uv, vec2(0.5));
    float strength = 0.5 / distanceToCenter - 1.0;
    vec4 texture = texture(uTextureSampler, uv);

    if(uMVP.useTexture != 1) {
        FragColor = vec4(1.0, 1.0, 1.0, strength);
    } else {
        FragColor = texture;
    }

}