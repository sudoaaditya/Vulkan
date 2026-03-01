#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outTexCoord;

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

void main(void) {

    vec3 fragCoord = gl_FragCoord.xyz;

    vec3 color = texture(uTextureSampler, outTexCoord).xyz;

    color += texture(uTextureSampler, vec2((fragCoord.x + 1.0), (fragCoord.y))).xyz;
    color += texture(uTextureSampler, vec2((fragCoord.x - 1.0), (fragCoord.y))).xyz;
    color += texture(uTextureSampler, vec2((fragCoord.x), (fragCoord.y + 1.0))).xyz;
    color += texture(uTextureSampler, vec2((fragCoord.x), (fragCoord.y - 1.0))).xyz;

    FragColor = vec4(color, 1.0);
}