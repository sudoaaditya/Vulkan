#version 450 core
#extension GL_ARB_separate_shader_objects : enable

const int WAVE_COUNT = 4;
const float PI = 3.14159265359;

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

// ---- Hash-based noise for detail normal perturbation ----
vec3 hash33(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453123) * 2.0 - 1.0;
}

float gradientNoise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(dot(hash33(i + vec3(0,0,0)), f - vec3(0,0,0)),
                       dot(hash33(i + vec3(1,0,0)), f - vec3(1,0,0)), u.x),
                   mix(dot(hash33(i + vec3(0,1,0)), f - vec3(0,1,0)),
                       dot(hash33(i + vec3(1,1,0)), f - vec3(1,1,0)), u.x), u.y),
               mix(mix(dot(hash33(i + vec3(0,0,1)), f - vec3(0,0,1)),
                       dot(hash33(i + vec3(1,0,1)), f - vec3(1,0,1)), u.x),
                   mix(dot(hash33(i + vec3(0,1,1)), f - vec3(0,1,1)),
                       dot(hash33(i + vec3(1,1,1)), f - vec3(1,1,1)), u.x), u.y), u.z);
}

// Compute detail normals via central differences in fragment shader
vec3 detailNormal(vec3 worldPos, float time) {
    float freq = uMVP.detailParams.y * 2.8;
    float speed = uMVP.detailParams.z * 0.6;
    float strength = uMVP.detailParams.x * 1.5;
    float eps = 0.04;

    vec3 p = worldPos * freq + vec3(0.0, 0.0, time * speed);

    float h  = gradientNoise(p);
    float hx = gradientNoise(p + vec3(eps, 0.0, 0.0));
    float hz = gradientNoise(p + vec3(0.0, 0.0, eps));

    // Second octave for micro-ripples
    vec3 p2 = worldPos * freq * 2.7 + vec3(time * speed * 0.7, 13.1, time * speed * 0.5);
    float h2  = gradientNoise(p2) * 0.35;
    float hx2 = gradientNoise(p2 + vec3(eps, 0.0, 0.0)) * 0.35;
    float hz2 = gradientNoise(p2 + vec3(0.0, 0.0, eps)) * 0.35;

    h += h2; hx += hx2; hz += hz2;

    float dhdx = (hx - h) / eps;
    float dhdz = (hz - h) / eps;

    return normalize(vec3(-dhdx * strength, 1.0, -dhdz * strength));
}

// Schlick Fresnel for water (IOR ~1.333)
float schlickFresnel(float cosTheta) {
    float R0 = 0.02; // ((1.333 - 1) / (1.333 + 1))^2
    return R0 + (1.0 - R0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// GGX specular distribution
float ggxSpecular(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 0.0001);
}

// ACES filmic tone mapping
vec3 acesTonemap(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main(void) {
    float time = uMVP.cameraPosition.w;
    vec3 normal = normalize(worldNormalOut);
    vec3 viewDirection = normalize(uMVP.cameraPosition.xyz - worldPositionOut);
    vec3 lightDirection = normalize(uMVP.sunDirection.xyz);
    float viewDist = length(uMVP.cameraPosition.xyz - worldPositionOut);

    // ---- Detail normal perturbation ----
    // Fade out detail normals at distance to avoid shimmer
    float detailFade = 1.0 - smoothstep(4.0, 18.0, viewDist);
    vec3 detailN = detailNormal(worldPositionOut, time);
    // Blend detail normals into the geometric normal in tangent-ish space
    normal = normalize(mix(normal, normalize(normal + (detailN - vec3(0,1,0)) * 0.6), detailFade));

    vec3 halfVec = normalize(lightDirection + viewDirection);
    vec3 reflectionDirection = reflect(-viewDirection, normal);

    // ---- Water color with depth-based absorption ----
    float baseMix = clamp((waveHeightOut + uMVP.shadingParams.x) * uMVP.shadingParams.y, 0.0, 1.0);
    vec3 deepColor = uMVP.depthColor.rgb;
    vec3 shallowColor = uMVP.surfaceColor.rgb;
    vec3 waterColor = mix(deepColor, shallowColor, baseMix);

    // ---- Subsurface scattering ----
    // Light transmitting through thin wave crests
    float sssWrap = max(dot(normal, -lightDirection) * 0.5 + 0.5, 0.0);
    float sssThin = clamp(1.0 - abs(waveHeightOut) * 3.0, 0.0, 1.0); // stronger on thin crests
    float sssViewAlign = pow(max(dot(viewDirection, -lightDirection), 0.0), 3.0);
    vec3 sssColor = shallowColor * 1.6 + vec3(0.0, 0.05, 0.02);
    vec3 sss = sssColor * sssWrap * sssThin * sssViewAlign * uMVP.lightingParams.w * 0.4;

    // ---- Fresnel ----
    float NdotV = max(dot(normal, viewDirection), 0.0);
    float fresnel = schlickFresnel(NdotV);
    // Boost fresnel using user param but keep it physically grounded
    fresnel = clamp(fresnel * uMVP.shadingParams.w, 0.0, 1.0);

    // ---- Sky reflection ----
    // Richer sky with horizon haze
    float reflY = reflectionDirection.y;
    vec3 skyColor = mix(
        uMVP.skyBottomColor.rgb,
        uMVP.skyTopColor.rgb,
        smoothstep(-0.05, 0.6, reflY)
    );
    // Horizon haze band
    float horizonHaze = exp(-abs(reflY) * 6.0) * 0.35;
    skyColor += uMVP.sunColor.rgb * horizonHaze;

    // Sun disk in reflection
    float sunDotRefl = max(dot(reflectionDirection, lightDirection), 0.0);
    vec3 sunDisk = uMVP.sunColor.rgb * uMVP.lightingParams.w * pow(sunDotRefl, 512.0) * 3.0;
    skyColor += sunDisk;

    // ---- Combine reflection and refraction ----
    vec3 reflectedColor = skyColor;
    vec3 refractedColor = waterColor;
    vec3 surfaceColor = mix(refractedColor, reflectedColor, fresnel);

    // ---- Diffuse ambient ocean light ----
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float ambientOcclusion = clamp(0.5 + 0.5 * normal.y, 0.0, 1.0); // troughs get darker
    vec3 ambient = waterColor * (0.06 * ambientOcclusion);

    // ---- GGX Specular ----
    float roughness = max(1.0 / sqrt(max(uMVP.lightingParams.x, 1.0)), 0.02);
    float spec = ggxSpecular(normal, halfVec, roughness) * diffuse;
    vec3 specColor = uMVP.sunColor.rgb * uMVP.lightingParams.w * spec * fresnel;

    // ---- Foam ----
    float normalTilt = 1.0 - clamp(normal.y, 0.0, 1.0);
    // Organic foam with noise turbulence
    float foamNoise = gradientNoise(worldPositionOut * 5.0 + vec3(time * 0.3)) * 0.15;
    float foamBase = waveHeightOut + normalTilt * 0.5 + foamNoise;
    float foam = smoothstep(
        uMVP.lightingParams.y,
        uMVP.lightingParams.y + 0.18,
        foamBase
    ) * uMVP.lightingParams.z;
    // Foam color slightly warm, not pure white
    vec3 foamColor = vec3(0.92, 0.95, 0.98);

    // ---- Assemble final color ----
    vec3 finalColor = surfaceColor + ambient + sss;
    finalColor += specColor;
    finalColor = mix(finalColor, foamColor, clamp(foam, 0.0, 0.75));

    // ---- Distance fog / atmospheric haze ----
    float fog = 1.0 - exp(-viewDist * viewDist * 0.0008);
    vec3 fogColor = mix(uMVP.skyBottomColor.rgb, uMVP.sunColor.rgb * 0.15, 0.3);
    finalColor = mix(finalColor, fogColor, clamp(fog, 0.0, 0.6));

    // ---- Tone mapping ----
    finalColor = acesTonemap(finalColor * 1.1);

    FragColor = vec4(finalColor, 1.0);
}
