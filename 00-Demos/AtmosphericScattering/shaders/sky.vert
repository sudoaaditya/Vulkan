#version 450 core
#extension GL_ARB_separate_shader_objects : enable

// Sean O'Neil's classic real-time atmospheric scattering (GPU Gems 2, Chapter 16).
// This shader integrates Rayleigh/Mie in-scattering along the view ray for the sky dome.
// It handles both "camera inside the atmosphere" and "camera in space" (branch on
// uMVP.sunDirection.w) instead of shipping as two separate SkyFromSpace/SkyFromAtmosphere
// programs -- the branch is uniform (not per-pixel), so there's no divergence cost.

layout(location = 0) in vec3 vPosition;

layout(binding = 0) uniform mvpMatrix {
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec4 cameraPosition;   // xyz = camera position, w = camera height
    vec4 sunDirection;     // xyz = normalized sun direction, w = cameraInSpace flag (0/1)
    vec4 invWavelength;    // 1/lambda^4 per channel (r,g,b)
    vec4 radii;            // innerRadius, outerRadius, scaleDepth, scale/scaleDepth
    vec4 kCoeffs;          // Kr, Km, ESun, g
    vec4 derivedCoeffs;    // Kr*ESun, Km*ESun, Kr*4PI, Km*4PI
    vec4 miscParams;       // exposure, ambientStrength, specularStrength, shininess (ground-only)
    vec4 groundColor;      // ground albedo (rgb), unused
} uMVP;

// Per-draw model matrix: ground and sky dome share one unit-sphere mesh, scaled here
// to kInnerRadius / kOuterRadius respectively (see buildCommandBuffers() in vk.cpp)
layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) out vec3 vRayleighColor;
layout(location = 1) out vec3 vMieColor;
layout(location = 2) out vec3 vDirection;

const int nSamples = 16;

// Approximates the integral of atmospheric density along a ray, as a function of the
// angle between the ray and the surface normal at the sample point. Fitted polynomial
// from the chapter -- avoids needing a precomputed 2D lookup table.
float scaleFn(float fCos, float fScaleDepth) {
    float x = 1.0 - fCos;
    return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

float getNearIntersection(vec3 pos, vec3 ray, float distanceSq, float radiusSq) {
    float B = 2.0 * dot(pos, ray);
    float C = distanceSq - radiusSq;
    float det = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(det));
}

void main() {
    vec3 v3Pos = (pc.model * vec4(vPosition, 1.0)).xyz; // sky dome vertex, radius = outerRadius
    vec3 v3CameraPos = uMVP.cameraPosition.xyz;
    float fCameraHeight = uMVP.cameraPosition.w;
    float fCameraHeight2 = fCameraHeight * fCameraHeight;

    float fInnerRadius = uMVP.radii.x;
    float fOuterRadius = uMVP.radii.y;
    float fOuterRadius2 = fOuterRadius * fOuterRadius;
    float fScaleDepth = uMVP.radii.z;
    float fScaleOverScaleDepth = uMVP.radii.w;
    float fScale = 1.0 / (fOuterRadius - fInnerRadius);

    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    vec3 v3Start;
    float fStartOffset;
    bool bCameraInSpace = uMVP.sunDirection.w > 0.5;

    if(bCameraInSpace) {
        // Camera outside the atmosphere: march starts where the ray enters the outer shell
        float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
        v3Start = v3CameraPos + v3Ray * fNear;
        fFar -= fNear;

        float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
        float fStartDepth = exp(-1.0 / fScaleDepth);
        fStartOffset = fStartDepth * scaleFn(fStartAngle, fScaleDepth);
    } else {
        // Camera inside the atmosphere: march starts at the camera itself
        v3Start = v3CameraPos;
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
        float fStartAngle = dot(v3Ray, v3Start) / fCameraHeight;
        fStartOffset = fDepth * scaleFn(fStartAngle, fScaleDepth);
    }

    float fSampleLength = fFar / float(nSamples);
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    vec3 v3InvWavelength = uMVP.invWavelength.xyz;
    float fKr4PI = uMVP.derivedCoeffs.z;
    float fKm4PI = uMVP.derivedCoeffs.w;
    vec3 v3LightPos = normalize(uMVP.sunDirection.xyz);

    vec3 v3FrontColor = vec3(0.0);

    for(int i = 0; i < nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = fStartOffset + fDepth * (scaleFn(fLightAngle, fScaleDepth) - scaleFn(fCameraAngle, fScaleDepth));
        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));

        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }

    float fKrESun = uMVP.derivedCoeffs.x;
    float fKmESun = uMVP.derivedCoeffs.y;

    vRayleighColor = v3FrontColor * (v3InvWavelength * fKrESun);
    vMieColor = v3FrontColor * fKmESun;
    vDirection = v3CameraPos - v3Pos;

    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * vec4(v3Pos, 1.0);
}
