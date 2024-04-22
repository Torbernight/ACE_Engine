#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec3 iPos;

layout(location=0) out vec3 oTexCoord;

#include "transform.glsl"

void main()
{
    mat4 transform = mat4(mat3(uView));
    oTexCoord = iPos;
    gl_Position = (uProjection * transform * vec4(iPos, 1.0)).xyww;
}