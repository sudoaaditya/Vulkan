#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;

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
    vec4 sunParams;
    vec4 skyParams;
    vec4 shadingParams;
    vec4 lightingParams;
    vec4 sphereParams; // x: 1/radius, y: max angle, z: sphere blend
    vec4 bronzeDarkColor;
    vec4 bronzeBrightColor;
} uMVP;

layout(location = 0) out vec3 worldPositionOut;
layout(location = 1) out vec3 worldNormalOut;
layout(location = 2) out float waveHeightOut;
layout(location = 3) out vec3 surfaceUpOut;     // surface up (no waves)
layout(location = 4) out vec3 tangentXOut;      // flat +X on the surface
layout(location = 5) out vec3 tangentZOut;      // flat +Z on the surface
layout(location = 6) out float sphereAngleOut;  // angle from north pole
layout(location = 7) out vec3 globeDirectionOut; // from sphere centre, for mask

// Classic Perlin 3D Noise 
// by Stefan Gustavson
//
vec4 permute(vec4 x) {
    return mod(((x*34.0)+1.0)*x, 289.0);
}

vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}
vec3 fade(vec3 t) {
    return t*t*t*(t*(t*6.0-15.0)+10.0);
}

float cnoise(vec3 P) {
    vec3 Pi0 = floor(P); // Integer part for indexing
    vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
    Pi0 = mod(Pi0, 289.0);
    Pi1 = mod(Pi1, 289.0);
    vec3 Pf0 = fract(P); // Fractional part for interpolation
    vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = Pi0.zzzz;
    vec4 iz1 = Pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    vec4 gx0 = ixy0 / 7.0;
    vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
    gx0 = fract(gx0);
    vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
    vec4 sz0 = step(gz0, vec4(0.0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    vec4 gx1 = ixy1 / 7.0;
    vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
    gx1 = fract(gx1);
    vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
    vec4 sz1 = step(gz1, vec4(0.0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

    vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
    vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
    vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
    vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
    vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
    vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
    vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
    vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

    vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);

    vec3 fade_xyz = fade(Pf0);
    vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
    vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
    return 2.2 * n_xyz;
}

vec3 sampleOceanSurface(vec2 surfacePoint) {
    vec3 displaced = vec3(surfacePoint, 0.0);
    float time = uMVP.cameraPosition.w;

    for(int i = 0; i < WAVE_COUNT; i++) {
        vec2 direction = normalize(uMVP.waveDirections[i].xy);
        float steepness = uMVP.waveDirections[i].z;
        float wavelength = max(uMVP.waveDirections[i].w, 0.001);
        float amplitude = uMVP.waveSettings[i].x;
        float speed = uMVP.waveSettings[i].y;
        float phaseOffset = uMVP.waveSettings[i].z;
        float waveNumber = (2.0 * PI) / wavelength;
        float phase = waveNumber * dot(direction, surfacePoint) - speed * time + phaseOffset;
        float waveSin = sin(phase);
        float waveCos = cos(phase);

        displaced.xy += direction * (steepness * amplitude * waveCos);
        displaced.z += amplitude * waveSin;
    }

    int detailLayers = int(clamp(uMVP.detailParams.w, 0.0, 4.0));
    for(int layer = 0; layer < detailLayers; layer++) {
        float octave = float(layer + 1);
        float octaveSpeed = uMVP.detailParams.z * (0.65 + 0.18 * octave);
        vec2 octaveFlow = normalize(uMVP.waveDirections[0].xy) * (time * octaveSpeed);
        vec2 octaveUV = surfacePoint * (uMVP.detailParams.y * octave) + octaveFlow + vec2(octave * 13.1, octave * 7.7);
        displaced.z += cnoise(vec3(octaveUV, time * octaveSpeed * 0.25)) * (uMVP.detailParams.x / octave);
    }

    return displaced;
}

// Wrap flat point (xy, height z) onto sphere of curvature k; k = 0 is flat, origin = north pole
vec3 bendOntoSphere(vec3 flatPosition) {
    float k = uMVP.sphereParams.x;
    float r = length(flatPosition.xy);
    if(k < 1e-5 || r < 1e-6) {
        return flatPosition;
    }

    vec2 direction = flatPosition.xy / r;
    float theta = r * k;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinHalf = sin(0.5 * theta);
    float height = flatPosition.z;

    // -2 sin^2(t/2) == cos(t) - 1, precise for tiny k
    return vec3(
        direction * (sinTheta / k + height * sinTheta),
        -2.0 * sinHalf * sinHalf / k + height * cosTheta
    );
}

void main (void) {
    vec2 surfacePoint = vPosition.xy;
    vec3 flatPosition = sampleOceanSurface(surfacePoint);
    vec3 localPosition = bendOntoSphere(flatPosition);

    // Adaptive epsilon: smaller step for tighter normals
    float eps = 0.05;
    vec3 localPositionDx = bendOntoSphere(sampleOceanSurface(surfacePoint + vec2(eps, 0.0)));
    vec3 localPositionDy = bendOntoSphere(sampleOceanSurface(surfacePoint + vec2(0.0, eps)));
    vec3 localNormal = normalize(cross(localPositionDx - localPosition, localPositionDy - localPosition));

    // Surface frame without waves (up / tangents)
    vec3 basePosition = bendOntoSphere(vec3(surfacePoint, 0.0));
    vec3 baseTangentX = bendOntoSphere(vec3(surfacePoint + vec2(eps, 0.0), 0.0)) - basePosition;
    vec3 baseTangentY = bendOntoSphere(vec3(surfacePoint + vec2(0.0, eps), 0.0)) - basePosition;
    vec3 baseUp = cross(baseTangentX, baseTangentY);

    mat3 normalMatrix = transpose(inverse(mat3(uMVP.modelMatrix)));
    vec4 worldPosition = uMVP.modelMatrix * vec4(localPosition, 1.0);

    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * worldPosition;

    worldPositionOut = worldPosition.xyz;
    worldNormalOut = normalize(normalMatrix * localNormal);
    waveHeightOut = flatPosition.z;
    surfaceUpOut = normalize(normalMatrix * baseUp);
    tangentXOut = normalize(mat3(uMVP.modelMatrix) * baseTangentX);
    tangentZOut = normalize(mat3(uMVP.modelMatrix) * -baseTangentY); // plane +Y = world -Z
    sphereAngleOut = length(surfacePoint) * uMVP.sphereParams.x;

    // Direction from sphere centre (0, 0, -1/k)
    vec3 localGlobeDirection = vec3(0.0, 0.0, 1.0);
    if(uMVP.sphereParams.x >= 1e-5) {
        localGlobeDirection = basePosition + vec3(0.0, 0.0, 1.0 / uMVP.sphereParams.x);
    }
    globeDirectionOut = normalize(mat3(uMVP.modelMatrix) * localGlobeDirection);
}
