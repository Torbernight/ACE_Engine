#version 460

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;

layout(location=0) out vec4 oColor;

void main()
{
    oColor = vec4(1.0);
}