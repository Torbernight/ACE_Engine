#version 460

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;

layout(location=0) out vec3 oPos;
layout(location=1) out vec2 oTexCoords;
layout(location=2) out vec3 oNormal;


layout(std140, binding=0) uniform Transforms
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
};

void main()
{
    vec4 pos = Model * vec4(iPos, 1.0);
    oPos = pos.xyz;
    gl_Position = Projection * View * pos;
    oNormal = normalize( transpose(inverse(mat3(Model))) * iNormal );
    oTexCoords = iTexCoords;
}
