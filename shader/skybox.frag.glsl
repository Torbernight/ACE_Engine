#version 460

layout(early_fragment_tests) in;

#extension GL_GOOGLE_include_directive : enable

layout(constant_id = 0) const int cubemap_binding = 0;

layout(location=0) in vec3 iTexCoord;

layout(location=0) out vec4 oAlbedo;
layout(location=1) out vec4 oNormal;
layout(location=2) out vec4 oPosition;
layout(location=3) out vec4 oAO_Roughness_Metallic;
layout(location=4) out vec4 oEmissive;

layout(binding = 0) uniform samplerCube skybox[cubemap_binding+1];

#define INF 1.0/0.0

void main()
{
    oAlbedo = texture(skybox[cubemap_binding], iTexCoord);
    oNormal =   vec4(vec3(0.0), 1.0);
    oPosition = vec4(vec3(INF), 1.0);
    oAO_Roughness_Metallic = vec4(1.0, 1.0, 0.0, 1.0);
    oEmissive = oAlbedo;
}