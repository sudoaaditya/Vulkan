#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform mvpMatrix {
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec4 cameraPosition;
    vec4 sunDirection;
    vec4 invWavelength;
    vec4 radii;
    vec4 kCoeffs;         // Kr, Km, ESun, g
    vec4 derivedCoeffs;
    vec4 miscParams;      // exposure, ambientStrength, specularStrength, shininess (ground-only)
    vec4 groundColor;
} uMVP;

layout(location = 0) in vec3 vRayleighColor;
layout(location = 1) in vec3 vMieColor;
layout(location = 2) in vec3 vDirection;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 v3LightPos = normalize(uMVP.sunDirection.xyz);
    vec3 v3Direction = normalize(vDirection);

    float fCos = dot(v3LightPos, v3Direction);
    float fCos2 = fCos * fCos;

    float g = uMVP.kCoeffs.w;
    float g2 = g * g;

    // Rayleigh phase function: symmetric scattering
    float fRayleighPhase = 0.75 * (1.0 + fCos2);
    // Henyey-Greenstein Mie phase function: strongly forward-scattering (bright sun halo)
    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(max(1.0 + g2 - 2.0 * g * fCos, 0.0001), 1.5);

    vec3 color = fRayleighPhase * vRayleighColor + fMiePhase * vMieColor;

    float exposure = uMVP.miscParams.x;
    color = vec3(1.0) - exp(-exposure * color);

    FragColor = vec4(color, 1.0);
}
