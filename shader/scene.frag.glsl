#version 460

#extension GL_GOOGLE_include_directive : enable

layout(constant_id = 0) const uint nMaterials = 1;

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;
layout(location=3) in vec3 iTangent;
layout(location=4) in vec3 iBitangent;

layout(location=0) out vec4 oAlbedo;
layout(location=1) out vec3 oNormal;
layout(location=2) out vec3 oPosition;
layout(location=3) out float oRoughness;

#include "uniform_blocks.glsl"

layout(binding=0) uniform sampler2D TextureArray[nMaterials*3];

vec3 get_normal()
{
    if (normal_index < 0)
        return iNormal;
    return mat3(iTangent, iBitangent, iNormal) * 
    (texture(TextureArray[normal_index], iTexCoords).xyz*2.0-1.0);
}

float get_roughness()
{
    if (roughness_index < 0)
        return 0.0;
    return texture(TextureArray[roughness_index], iTexCoords).r;

}

void main()
{
    vec3 albedo = texture(TextureArray[albedo_index], iTexCoords).rgb;
    oAlbedo = vec4(albedo, 1.0);
    oNormal = normalize(get_normal());
    oRoughness = get_roughness();
    oPosition = iPos;
}