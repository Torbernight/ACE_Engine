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

layout(binding=0) uniform sampler2D TextureArray[nTextures];

vec3 get_albedo()
{
    if (uAlbedo_index < 0)
        return vec3(0.0);
    return texture(TextureArray[uAlbedo_index], iTexCoords).rgb;
}

vec3 get_normal()
{
    if (uNormal_index < 0)
        return iNormal;
    return mat3(iTangent, iBitangent, iNormal) * 
    (texture(TextureArray[uNormal_index], iTexCoords).xyz*2.0-1.0);
}

vec3 get_AO_roughness_metallic()
{
    if (uAO_roughness_metallic_index < 0)
        return vec3(1.0, 0.0, 0.0);
    return texture(TextureArray[uAO_roughness_metallic_index], iTexCoords).rgb;
}

vec3 get_emissive()
{
    if (uEmissive_index < 0)
        return vec3(0);
    return texture(TextureArray[uEmissive_index], iTexCoords).rgb;
}

void main()
{
    ivec2 index = ivec2(gl_FragCoord.xy); 
    index = index%ivec2(4);
    float A = bayerMat[index.y][index.x];

    float alpha = step(A, uAlpha);
    if(alpha == 0){
        return;
    }

    oAlbedo = vec4(get_albedo(), alpha);
    oNormal = vec4(normalize(get_normal()), alpha);
    oAO_Roughness_Metallic = vec4(get_AO_roughness_metallic(), alpha);
    oPosition = vec4(iPos, alpha);
    oEmissive = vec4(get_emissive(), alpha);
    if (uGlow != 0) {
        oEmissive = vec4(1.0, 1.0, 1.0, alpha);
    }
}

