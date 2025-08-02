#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(location = 1) in vec4 fPosition;

void main(void) {

    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = vec4(fPosition.x, fPosition.y, fPosition.z, 1.0);
}
