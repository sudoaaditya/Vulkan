#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in float outViewPosZ;

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void) {

    vec4 tex = texture(uTextureSampler, outTexCoord);

    float fadeNear = 50.0;    
    float fadeFar  = 800.0;  

    float fade = 1.0 - clamp((outViewPosZ - fadeNear) / (fadeFar - fadeNear), 0.0, 1.0);

    FragColor = vec4(tex.rgb, tex.a * fade);
}