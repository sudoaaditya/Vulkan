#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 color;
} uMVP;

void main(void) {

    FragColor = uMVP.color;
}
