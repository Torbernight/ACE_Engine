#version 460

layout(constant_id = 0) const uint nMaterials = 1;

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;
layout(location=3) in vec3 iTangent;
layout(location=4) in vec3 iBitangent;

layout(location=0) out vec4 oAlbedo;
layout(location=1) out vec3 oNormal;
layout(location=2) out vec3 oPosition;

layout(std140, binding=0) uniform Transforms
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
};

struct Material
{
    uint albedo;
    uint normal;
    uint ao;
    uint junk;
};

layout(std140, binding = 3) uniform Materials
{
    uint material_index;
    Material uMaterials[nMaterials];
};

layout(binding=0) uniform sampler2D Albedo[nMaterials];

void main()
{
    vec3 albedo = texture(Albedo[material_index], iTexCoords).rgb;
    oAlbedo = vec4(albedo, 1.0);
    oNormal = iNormal;
    oPosition = iPos;
}