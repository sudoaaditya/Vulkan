#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

layout(binding = 0) uniform myUniformData {
    // Matrices
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    // Lights
    vec4 lightAmbient;
    vec4 lightDiffuse;
    vec4 lightSpecular;
    vec4 lightPosition;
    // Key Pressed Uniform
    uint lKeyPressed;
} uMyUniforms;

layout(location = 0) out vec3 out_tNormal;
layout(location = 1) out vec3 out_viewerVector;
layout(location = 2) out vec3 out_lightDirection;

void main (void) {
    // code
    if(uMyUniforms.lKeyPressed == 1) {
        vec4 eyeCoords = uMyUniforms.viewMatrix * uMyUniforms.modelMatrix * vPosition;
        out_tNormal = mat3(uMyUniforms.viewMatrix * uMyUniforms.modelMatrix) * vNormal;
        out_lightDirection = vec3(uMyUniforms.lightPosition - eyeCoords);
        out_viewerVector = vec3(-eyeCoords.xyz);
    }
    gl_Position = uMyUniforms.projectionMatrix * uMyUniforms.viewMatrix * uMyUniforms.modelMatrix * vPosition;
}
