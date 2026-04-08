#version 450 core
#extension GL_ARB_separate_shader_objects : enable

const int WAVE_COUNT = 4;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec4 cameraPosition;
    vec4 waveDirections[WAVE_COUNT];
    vec4 waveSettings[WAVE_COUNT];
    vec4 detailParams;
    vec4 depthColor;
    vec4 surfaceColor;
    vec4 skyBottomColor;
    vec4 skyTopColor;
    vec4 sunDirection;
    vec4 sunColor;
    vec4 shadingParams;
    vec4 lightingParams;
} uMVP;

layout(location = 0) in vec3 worldPositionOut;
layout(location = 1) in vec3 worldNormalOut;
layout(location = 2) in float waveHeightOut;

layout(location = 0) out vec4 FragColor;

void main(void) {
    vec3 normal = normalize(worldNormalOut);
    vec3 viewDirection = normalize(uMVP.cameraPosition.xyz - worldPositionOut);
    vec3 lightDirection = normalize(uMVP.sunDirection.xyz);
    vec3 reflectionDirection = reflect(-viewDirection, normal);
    float baseMix = clamp((waveHeightOut + uMVP.shadingParams.x) * uMVP.shadingParams.y, 0.0, 1.0);
    float fresnel = pow(1.0 - max(dot(normal, viewDirection), 0.0), max(uMVP.shadingParams.z, 1.0));
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float specular = pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), max(uMVP.lightingParams.x, 1.0)) * diffuse;
    float foam = smoothstep(
        uMVP.lightingParams.y,
        uMVP.lightingParams.y + 0.25,
        waveHeightOut + (1.0 - clamp(normal.y, 0.0, 1.0)) * 0.45
    ) * uMVP.lightingParams.z;

    vec3 waterColor = mix(uMVP.depthColor.rgb, uMVP.surfaceColor.rgb, baseMix);
    vec3 skyColor = mix(
        uMVP.skyBottomColor.rgb,
        uMVP.skyTopColor.rgb,
        smoothstep(-0.15, 0.85, reflectionDirection.y)
    );
    vec3 reflectedColor = mix(waterColor, skyColor, clamp(fresnel * uMVP.shadingParams.w, 0.0, 1.0));
    vec3 sunTint = uMVP.sunColor.rgb * uMVP.lightingParams.w;
    vec3 finalColor = reflectedColor;

    finalColor += waterColor * (0.08 + diffuse * 0.12);
    finalColor += sunTint * specular;
    finalColor = mix(finalColor, vec3(1.0), clamp(foam, 0.0, 0.85));
    FragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
}
