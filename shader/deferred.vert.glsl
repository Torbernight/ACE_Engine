#version 460

layout(location = 0) in vec2 iPos;
layout(location = 1) in vec2 iTexCoords;

layout(location = 0) out vec2 oTexCoords;

void main()
{
        gl_Position = vec4(iPos, 0.0, 1.0);
        oTexCoords = iTexCoords;
}