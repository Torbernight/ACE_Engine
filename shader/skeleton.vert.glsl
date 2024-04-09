#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;
layout(location=3) in vec3 iTangent;
layout(location=4) in vec3 iBitangent;

layout(location=5) in vec4  iBoneWeights;
layout(location=7) in ivec4 iBoneIndex;

layout(location=0) out vec3 oPos;
layout(location=1) out vec2 oTexCoords;
layout(location=2) out vec3 oNormal;
layout(location=3) out vec3 oTangent;
layout(location=4) out vec3 oBitangent;

layout(constant_id = 0) const int nBones = 1;

layout(binding = 5, std140) uniform Skeleton
{
        mat4 uBones[nBones];
};

#include "uniform_blocks.glsl"

void main()
{
    vec4 pos = vec4(iPos, 1.0);
    vec4 totalPos = vec4(0);
    mat4 transform = mat4(1);
    vec3 totalNormal = vec3(0);
    vec3 totalTangent = vec3(0);
    vec3 totalBitangent = vec3(0);

    for(int n = 0; n < 4; n++)
    {
        int bone_index = iBoneIndex[n];
        if (bone_index == -1)
            continue;
        if (bone_index >= nBones){
            totalPos = pos;
            break;
        }
        totalPos       += iBoneWeights[n] * uBones[bone_index] * pos;
        totalNormal    += iBoneWeights[n] * mat3(uBones[bone_index]) * iNormal;
        totalTangent   += iBoneWeights[n] * mat3(uBones[bone_index]) * iTangent;
        totalBitangent += iBoneWeights[n] * mat3(uBones[bone_index]) * iBitangent;
    }

    totalNormal = normalize(totalNormal);
    totalTangent = normalize(totalTangent);
    totalBitangent = normalize(totalBitangent);
    
    mat3 normalTrans = transpose(inverse(mat3(Model)));
    gl_Position = Projection * View * pos;
    oNormal    = normalize( normalTrans * totalNormal );
    oTangent   = normalize( normalTrans * totalTangent);
    oBitangent = normalize( normalTrans * totalBitangent);
    oTexCoords = iTexCoords;
}
