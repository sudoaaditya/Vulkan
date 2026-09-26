#version 450 core
#extension GL_ARB_separate_shader_objects : enable

// Same in-scattering integral as sky.vert, evaluated from the camera to a ground vertex
// instead of to the sky dome. Additionally tracks the out-scattering attenuation at the
// ground point itself, used in the fragment shader to darken direct sunlight on the terrain.

layout(location = 0) in vec3 vPosition;

layout(binding = 0) uniform mvpMatrix {
    mat4 viewMatrix;
    mat4 projectionMatrix;

    vec4 cameraPosition;
    vec4 sunDirection;
    vec4 invWavelength;
    vec4 radii;
    vec4 kCoeffs;
    vec4 derivedCoeffs;
    vec4 miscParams; // exposure, ambientStrength, specularStrength, shininess
    vec4 groundColor;
} uMVP;

// Per-draw model matrix: ground and sky dome share one unit-sphere mesh, scaled here
// to kInnerRadius / kOuterRadius respectively (see buildCommandBuffers() in vk.cpp)
layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) out vec3 vRayleighColor;
layout(location = 1) out vec3 vAttenuate;
layout(location = 2) out vec3 vNormal;
layout(location = 3) out vec2 vTexCoord;

const int nSamples = 16;
const float PI = 3.14159265359;

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
    vec3 v3Pos = (pc.model * vec4(vPosition, 1.0)).xyz; // ground vertex, radius = innerRadius
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
        float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
        v3Start = v3CameraPos + v3Ray * fNear;
        fFar -= fNear;

        float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
        float fStartDepth = exp(-1.0 / fScaleDepth);
        fStartOffset = fStartDepth * scaleFn(fStartAngle, fScaleDepth);
    } else {
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
    vec3 v3Attenuate = vec3(1.0);

    for(int i = 0; i < nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = fStartOffset + fDepth * (scaleFn(fLightAngle, fScaleDepth) - scaleFn(fCameraAngle, fScaleDepth));
        v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));

        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }

    float fKrESun = uMVP.derivedCoeffs.x;
    float fKmESun = uMVP.derivedCoeffs.y;

    // Ambient skylight added to the ground -- not phase-split since it isn't view dependent
    vRayleighColor = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
    vAttenuate = v3Attenuate; // attenuation of direct sunlight reaching this point
    vNormal = normalize(v3Pos);

    // Equirectangular mapping (longitude/latitude) from the local sphere direction -- same
    // convention gluSphere() uses for earthmap1k.jpg in the reference GPU Gems demo, except the
    // "pole" axis is X here instead of Y: the free camera always starts (and mostly stays) near
    // local +Y, which is exactly the singular/distorted pole of a Y-axis mapping -- putting the
    // pole on X instead means the camera's default vantage lands on an ordinary equatorial band.
    vec3 localDir = normalize(vPosition);
    vTexCoord = vec2(0.5 + atan(localDir.z, localDir.y) / (2.0 * PI), 0.5 - asin(clamp(localDir.x, -1.0, 1.0)) / PI);

    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * vec4(v3Pos, 1.0);
}
