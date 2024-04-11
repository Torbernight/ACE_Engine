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
layout(location=3) out vec2 oRoughness_Metallic;
layout(location=4) out vec3 oEmissive;

#include "materials.glsl"

layout(binding=0) uniform sampler2D TextureArray[nMaterials*4];

vec3 get_albedo()
{
    if (albedo_index < 0)
        return vec3(0.0);
    return texture(TextureArray[albedo_index], iTexCoords).rgb;
}

vec3 get_normal()
{
    if (normal_index < 0)
        return iNormal;
    return mat3(iTangent, iBitangent, iNormal) * 
    (texture(TextureArray[normal_index], iTexCoords).xyz*2.0-1.0);
}

vec2 get_roughness_metallic()
{
    if (roughness_metallic_index < 0)
        return vec2(0.0);
    return texture(TextureArray[roughness_metallic_index], iTexCoords).gb;
}

vec3 get_emissive()
{
    if (emissive_index < 0)
        return vec3(0);
    return texture(TextureArray[emissive_index], iTexCoords).rgb;
}

void main()
{
    oAlbedo = vec4(get_albedo(), 1.0);
    oNormal = normalize(get_normal());
    oRoughness_Metallic = get_roughness_metallic();
    oPosition = iPos;
    oEmissive = get_emissive();
}