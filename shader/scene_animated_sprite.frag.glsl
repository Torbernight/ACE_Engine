#version 460

#extension GL_GOOGLE_include_directive : enable

layout(constant_id = 0) const uint nTextures = 1;

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;
layout(location=3) in vec3 iTangent;
layout(location=4) in vec3 iBitangent;

layout(location=0) out vec4 oAlbedo;
layout(location=1) out vec4 oNormal;
layout(location=2) out vec4 oPosition;
layout(location=3) out vec4 oAO_Roughness_Metallic;
layout(location=4) out vec4 oEmissive;

const mat4 bayerMat = mat4(
    0.0, 0.75, 0.1875, 0.9375,
    0.75, 0.25, 0.6875, 0.4375,
    0.1875, 0.6875, 0.0625, 0.8125,
    0.9375, 0.4375, 0.8125, 0.3125
);

#include "materials.glsl"
#include "uniform_blocks.glsl"

layout(binding=0) uniform sampler2DArray TextureArray[nTextures];

vec3 get_sprite()
{
    float frame = uTime * uFPS;
    if (albedo_index < 0)
        return vec3(0.0);
    return texture(TextureArray[albedo_index], vec3(iTexCoords, frame)).rgb;
}

void main()
{
    ivec2 index = ivec2(gl_FragCoord.xy); 
    index = index%ivec2(4);
    float A = bayerMat[index.y][index.x];

    if(step(alpha, A) == 1.0){
        discard;
    }
    
    oAlbedo = vec4(get_sprite(), 1.0);
    oNormal = vec4(iNormal, 1.0);
    oAO_Roughness_Metallic = vec4(0);
    oPosition = vec4(iPos, 1.0);
    oEmissive = oAlbedo;
}