#version 460

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;

layout(location=0) out vec4 oColor;

layout(std140, binding=0) uniform Transforms
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
};

layout(std140, binding=1) uniform Camera
{
    vec4 Position;
    vec4 Front;
};

layout(binding=0) uniform sampler2D Albedo;

const float bright = 64;

void main()
{
    vec3 pos = Position.xyz;
    float d = distance(pos, iPos);
    vec3 dir = normalize(pos - iPos);
    vec3 h = normalize(iNormal-Front.xyz);

    float decay = 1.0/(1.0+0.025*d+0.125*d*d);

    float diff = decay*max(0.0, dot(iNormal, dir));
    float spec = decay*max(0.0, pow(dot(h,dir), bright));
    
    vec3 albedo = texture(Albedo, iTexCoords).rgb;

    vec3 col = albedo*(diff)+vec3(1.0)*spec;
    oColor = vec4(col, 1.0);
}