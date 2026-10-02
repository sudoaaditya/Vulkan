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
    vec4 sunDirection;   // xyz: direction TOWARDS the sun (normalized)
    vec4 sunColor;       // rgb: sun color, a: glow/halo strength around the sun
    vec4 sunParams;      // x: glitter strength, y: sky ambient strength, z: subsurface strength
    vec4 skyParams;      // x: horizon haze, y: fog density, w: sky exposure
    vec4 shadingParams;
    vec4 lightingParams; // x: specular power, y: foam height, z: foam intensity, w: sun intensity
    vec4 sphereParams;   // x: 1/radius, y: max angle, z: sphere blend
    vec4 bronzeDarkColor;   // rgb: troughs, a: roughness
    vec4 bronzeBrightColor; // rgb: crests, a: bronze blend
} uMVP;

layout(location = 0) in vec3 worldPositionOut;
layout(location = 1) in vec3 worldNormalOut;
layout(location = 2) in float waveHeightOut;
layout(location = 3) in vec3 surfaceUpOut;
layout(location = 4) in vec3 tangentXOut;
layout(location = 5) in vec3 tangentZOut;
layout(location = 6) in float sphereAngleOut;
layout(location = 7) in vec3 globeDirectionOut;

// Ocean mask in globe UVs (white = ocean)
layout(binding = 1) uniform sampler2D uOceanMask;

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

// Day sky for reflections and fog; keep in sync with the sky pass
vec3 sampleDaySky(vec3 direction) {
    vec3 dir = normalize(direction);
    // Below-horizon rays get horizon colour
    float elevation = max(dir.y, 0.0);

    vec3 sky = mix(
        uMVP.skyBottomColor.rgb,
        uMVP.skyTopColor.rgb,
        smoothstep(0.0, 1.0, pow(elevation, 0.45))
    );

    // Bright, slightly desaturated haze band hugging the horizon
    float haze = exp(-elevation * 9.0) * clamp(uMVP.skyParams.x, 0.0, 1.0);
    sky = mix(sky, uMVP.skyBottomColor.rgb, haze);

    // Forward scattering: sky warms and brightens around the sun
    vec3 sunDir = normalize(uMVP.sunDirection.xyz);
    float sunDot = max(dot(dir, sunDir), 0.0);
    vec3 sunLight = uMVP.sunColor.rgb * uMVP.lightingParams.w;
    float glow = clamp(uMVP.sunColor.a, 0.0, 1.0);
    sky += sunLight * glow * (pow(sunDot, 6.0) * 0.04 + pow(sunDot, 48.0) * 0.15);

    return sky * clamp(uMVP.skyParams.w, 0.2, 2.0);
}

void main(void) {
    // Trim plane past the south pole
    if(sphereAngleOut > uMVP.sphereParams.y) {
        discard;
    }

    float time = uMVP.cameraPosition.w;
    float sphereBlend = uMVP.sphereParams.z;

    // Cut out land; same UV mapping as the Atlas globe (north at v = 1)
    vec3 globeDirection = normalize(globeDirectionOut);
    vec2 globeUV = vec2(
        fract(atan(-globeDirection.z, globeDirection.x) / (2.0 * PI) + 0.75),
        asin(clamp(globeDirection.y, -1.0, 1.0)) / PI + 0.5
    );
    float oceanAmount = textureLod(uOceanMask, globeUV, 0.0).r;
    // Land dissolves in near the end of the wrap
    float landReveal = smoothstep(0.6, 1.0, sphereBlend);
    float dissolve = 0.5 + 0.5 * gradientNoise(globeDirection * 6.0);
    if(oceanAmount < 0.5 && landReveal > dissolve * 0.9 + 0.05) {
        discard;
    }
    vec3 surfaceUp = normalize(surfaceUpOut);
    vec3 normal = normalize(worldNormalOut);
    vec3 viewDirection = normalize(uMVP.cameraPosition.xyz - worldPositionOut);
    vec3 lightDirection = normalize(uMVP.sunDirection.xyz);
    float viewDist = length(uMVP.cameraPosition.xyz - worldPositionOut);
    vec3 sunLight = uMVP.sunColor.rgb * uMVP.lightingParams.w;

    // ---- Detail normal perturbation ----
    // Fade out detail normals at distance to avoid shimmer
    float detailFade = 1.0 - smoothstep(4.0, 18.0, viewDist);
    vec3 detailN = detailNormal(worldPositionOut, time);
    // Detail normals mapped onto surface tangents
    vec3 detailOffset = normalize(tangentXOut) * detailN.x + normalize(tangentZOut) * detailN.z;
    normal = normalize(mix(normal, normalize(normal + detailOffset * 0.6), detailFade));

    vec3 halfVec = normalize(lightDirection + viewDirection);
    vec3 reflectionDirection = reflect(-viewDirection, normal);
    float NdotL = max(dot(normal, lightDirection), 0.0);
    float NdotV = max(dot(normal, viewDirection), 0.0);

    // ---- Water color with depth-based absorption ----
    float baseMix = clamp((waveHeightOut + uMVP.shadingParams.x) * uMVP.shadingParams.y, 0.0, 1.0);
    vec3 deepColor = uMVP.depthColor.rgb;
    vec3 shallowColor = uMVP.surfaceColor.rgb;
    vec3 waterColor = mix(deepColor, shallowColor, baseMix);

    // ---- Water body lit by sky + sun ----
    float normalUp = dot(normal, surfaceUp);
    float ambientOcclusion = clamp(0.5 + 0.5 * normalUp, 0.0, 1.0); // troughs get darker
    vec3 skyAmbient = mix(uMVP.skyBottomColor.rgb, uMVP.skyTopColor.rgb, 0.5) * uMVP.sunParams.y;
    vec3 refractedColor = waterColor * (skyAmbient * ambientOcclusion + sunLight * (0.06 + 0.10 * NdotL));

    // ---- Subsurface scattering ----
    // Sunlight transmitting through thin wave crests when looking towards the sun
    float sssWrap = max(dot(normal, -lightDirection) * 0.5 + 0.5, 0.0);
    float sssThin = clamp(1.0 - abs(waveHeightOut) * 3.0, 0.0, 1.0); // stronger on thin crests
    float sssViewAlign = pow(max(dot(viewDirection, -lightDirection), 0.0), 4.0);
    float sssCrest = clamp(waveHeightOut * 4.0 + 0.5, 0.0, 1.0); // crests, not troughs
    vec3 sssColor = shallowColor * 1.6 + vec3(0.0, 0.08, 0.04);
    vec3 sss = sssColor * sunLight * sssWrap * sssThin * sssViewAlign * sssCrest * uMVP.sunParams.z;

    // ---- Fresnel ----
    float fresnel = schlickFresnel(NdotV);
    // Boost fresnel using user param but keep it physically grounded
    fresnel = clamp(fresnel * uMVP.shadingParams.w, 0.0, 1.0);

    // ---- Daytime sky reflection ----
    vec3 reflectedColor = sampleDaySky(reflectionDirection);

    // ---- Combine reflection and refraction ----
    vec3 surfaceColor = mix(refractedColor, reflectedColor, fresnel);

    // ---- Sun specular: sharp glitter + broad "sun road" ----
    float specFresnel = schlickFresnel(max(dot(halfVec, viewDirection), 0.0));
    float specNorm = NdotL / (4.0 * max(NdotV, 0.15));
    float roughness = max(1.0 / sqrt(max(uMVP.lightingParams.x, 1.0)), 0.02);
    float glitter = min(ggxSpecular(normal, halfVec, roughness) * specFresnel * specNorm, 25.0);
    float sunRoad = ggxSpecular(normal, halfVec, 0.38) * specFresnel * specNorm;
    vec3 specColor = sunLight * (glitter * uMVP.sunParams.x + sunRoad * 0.025);

    // ---- Foam ----
    float normalTilt = 1.0 - clamp(normalUp, 0.0, 1.0);
    // Organic foam with noise turbulence
    float foamNoise = gradientNoise(worldPositionOut * 5.0 + vec3(time * 0.3)) * 0.15;
    float foamBase = waveHeightOut + normalTilt * 0.2 + foamNoise;
    float foam = smoothstep(
        uMVP.lightingParams.y,
        uMVP.lightingParams.y + 0.12,
        foamBase
    ) * uMVP.lightingParams.z;
    float foamMask = clamp(foam, 0.0, 0.45);
    // Sunlit foam: white, picks up sky ambient and direct sun
    vec3 foamColor = vec3(0.92, 0.95, 0.98) * (skyAmbient * 0.6 + sunLight * 0.07 * (0.35 + 0.65 * NdotL));

    // ---- Assemble final color ----
    vec3 finalColor = surfaceColor + sss;
    finalColor = mix(finalColor, foamColor, foamMask);
    finalColor += specColor * (1.0 - foamMask);

    // ---- Bronze (metal like the Atlas model) ----
    float bronzeBlend = uMVP.bronzeBrightColor.a;
    if(bronzeBlend > 0.0) {
        float bronzeRoughness = clamp(uMVP.bronzeDarkColor.a, 0.05, 1.0);

        // Dark patina in troughs, bright crests, mottling
        float cavity = clamp((waveHeightOut + 0.04) * 8.0, 0.0, 1.0) * ambientOcclusion;
        float mottle = 0.5 + 0.5 * gradientNoise(worldPositionOut * 3.0);
        vec3 bronzeBase = mix(uMVP.bronzeDarkColor.rgb, uMVP.bronzeBrightColor.rgb, clamp(cavity * 0.8 + mottle * 0.25, 0.0, 1.0));

        // Metal: no diffuse, F0 = base colour; roughness damps white grazing edge
        float grazing = pow(1.0 - NdotV, 5.0) * (1.0 - bronzeRoughness) * 0.5;
        vec3 bronzeFresnel = mix(bronzeBase, vec3(1.0), grazing);

        // Blurred sky reflection, brightness only
        vec3 bronzeEnv = mix(sampleDaySky(reflectionDirection), skyAmbient, bronzeRoughness);
        float bronzeEnvLight = dot(bronzeEnv, vec3(0.299, 0.587, 0.114));
        vec3 bronzeColor = bronzeFresnel * bronzeEnvLight * (0.35 + 0.65 * ambientOcclusion);

        // Sun highlight + sheen
        float bronzeSpec = ggxSpecular(normal, halfVec, bronzeRoughness) * NdotL / (4.0 * max(NdotV, 0.15));
        bronzeColor += sunLight * bronzeBase * (bronzeSpec * 0.35 + NdotL * 0.05);

        // Noisy fade front
        float bronzeFront = 0.5 + 0.5 * gradientNoise(worldPositionOut * 0.35);
        float bronzeAmount = smoothstep(bronzeFront * 0.6, bronzeFront * 0.6 + 0.4, bronzeBlend);
        finalColor = mix(finalColor, bronzeColor, bronzeAmount);
    }

    // ---- Distance fog / aerial perspective ----
    // Fade into horizon sky colour
    vec3 viewRay = -viewDirection;
    vec3 horizonDir = normalize(vec3(viewRay.x, 0.02, viewRay.z));
    vec3 fogColor = sampleDaySky(horizonDir);
    float fog = 1.0 - exp(-viewDist * viewDist * uMVP.skyParams.y * 0.001);
    // No horizon fog on the globe
    fog *= 1.0 - sphereBlend;
    finalColor = mix(finalColor, fogColor, clamp(fog, 0.0, 0.95));

    // ---- Tone mapping ----
    finalColor = acesTonemap(finalColor * 0.9);

    FragColor = vec4(finalColor, 1.0);
}
