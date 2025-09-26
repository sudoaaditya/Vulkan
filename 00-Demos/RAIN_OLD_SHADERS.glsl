#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D uTextureSampler;

layout(location = 0) out vec4 FragColor;

// void main()	{
//     vec2 st = gl_PointCoord.xy;
//     vec4 normalTexture = texture(uTextureSampler, st);

//     // calculate normal from texture
//     vec3 normal = vec3(normalTexture.rg * 2.0 - 1.0, 0.0);
//     normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
//     normal = normalize(normal);

//     float disc = length(vec2(0.5) - st);
//     float alpha = smoothstep(0.2, 0.1, disc);

//     // calculate lights
//     vec3 lightPosition = vec3(1.0);
//     float diffuse = max(0.0, dot(normal, normalize(lightPosition)));

//     vec3 color = vec3(0.136, 0.559, 0.832);
//     vec3 color1 = vec3(0.79, 0.89, 0.91);

//     FragColor = vec4(color1, alpha * diffuse);
// }

void main() {
    float distanceToCenter = distance(gl_PointCoord, vec2(0.5));
    float strength = 0.5/distanceToCenter - 1.0;
    vec4 texture = texture(uTextureSampler, gl_PointCoord);

    // if (uSimple) {
        FragColor = vec4 ( 1.0,1.0,1.0,strength );
    // } else {
        // FragColor = texture;
    // }
    
}



//! VERTEX

#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec4 aRandom; // Random attribute

float PI = 3.141592653589793238;

layout(binding = 0) uniform mvpMatrix {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;

    // for snow
    float radius;
    float size;
    float time;
    int useTexture;
} uMVP;

void main (void) {

    // vec3 pos = vPosition.xyz;
    // float progress = fract(uMVP.time * 0.2 + aRandom.x);

    // float radius = 0.3 + aRandom.z * 0.2;
    // float cx = radius * cos(aRandom.y * PI * 2.0 * uMVP.time * 0.1 + aRandom.z * 7.0);
    // float cy = radius * sin(aRandom.y * PI * 2.0 * uMVP.time * 0.1 + aRandom.z * 7.0);

    // pos += vec3(cx, cy, - 20.0 * progress);
    // if(pos.z < -20.0) {
    //     pos.z += 20.0;
    // }

    // vec4 mvPosition = uMVP.viewMatrix * uMVP.modelMatrix * vec4( pos, 1.0 );

    // gl_PointSize = 150.0 * ( 1.0 / - mvPosition.z );
    // // gl_Position = uMVP.projectionMatrix * mvPosition;

    vec4 modelPosition = uMVP.modelMatrix * vPosition ;
    modelPosition.y =  mod(modelPosition.y - uMVP.time * 0.2,6.0);

    modelPosition.z = mod(modelPosition.z + (sin((uMVP.time + modelPosition.x )*0.5))* uMVP.radius * 0.8,7.0)-3.5;
    modelPosition.x = mod(modelPosition.x + (cos((uMVP.time + modelPosition.z )*0.5))* uMVP.radius,7.0)-3.5;

    vec4 viewPosition = uMVP.viewMatrix * modelPosition ;
    vec4 projectionPosition = uMVP.projectionMatrix * viewPosition;

    gl_Position = projectionPosition;
    gl_PointSize = uMVP.size * ( 1.0 / - viewPosition.z);
}