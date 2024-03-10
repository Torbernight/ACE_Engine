#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;

layout(location=0) out vec4 oColor;

const vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0));

void main()
{
    float diff = max(0.0, dot(iNormal, light_dir));
    oColor = vec4(vec3(iTexCoords, 0.0) * diff, 1.0);
}