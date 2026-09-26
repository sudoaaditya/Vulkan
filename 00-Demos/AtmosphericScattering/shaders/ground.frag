#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform mvpMatrix {
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec4 cameraPosition;
    vec4 sunDirection;
    vec4 invWavelength;
    vec4 radii;
    vec4 kCoeffs;
    vec4 derivedCoeffs;
    vec4 miscParams;      // exposure, ambientStrength, specularStrength, shininess
    vec4 groundColor;
} uMVP;

layout(binding = 1) uniform sampler2D groundTexture;

layout(location = 0) in vec3 vRayleighColor;
layout(location = 1) in vec3 vAttenuate;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uMVP.sunDirection.xyz);
    // Ground vertices sit exactly at radius = innerRadius along their own normal direction,
    // so the world position can be reconstructed without a dedicated varying.
    vec3 worldPos = normal * uMVP.radii.x;
    vec3 viewDir = normalize(uMVP.cameraPosition.xyz - worldPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotH = max(dot(normal, halfDir), 0.0);
    vec3 earthColor = texture(groundTexture, vTexCoord).rgb * uMVP.groundColor.rgb;

    float ambientStrength = uMVP.miscParams.y;
    float specularStrength = uMVP.miscParams.z;
    float shininess = uMVP.miscParams.w;

    // Classic per-fragment Blinn-Phong ambient/diffuse/specular (see
    // 10-Lights/03-PerFragmentSphere/shaders/shader.frag), using the sun as the light source.
    // The ambient term is a flat floor -- not multiplied by attenuation/NdotL -- so the planet
    // stays visibly lit even on the night side or when the scattering integral itself is tiny
    // (e.g. viewed from far in space, or at grazing angles near the horizon).
    vec3 ambient = earthColor * ambientStrength;
    vec3 diffuse = earthColor * NdotL;
    vec3 specular = vec3(specularStrength) * pow(NdotH, shininess) * step(0.0, NdotL);

    // Direct sunlight (diffuse+specular) attenuated by out-scattering along the view path,
    // plus ambient in-scattered skylight, plus the flat ambient floor above.
    vec3 color = (diffuse + specular) * vAttenuate + vRayleighColor + ambient;

    float exposure = uMVP.miscParams.x;
    color = vec3(1.0) - exp(-exposure * color);

    FragColor = vec4(color, 1.0);
}
