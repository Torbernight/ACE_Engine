#version 460

layout(constant_id = 0) const uint nLights = 1;
layout(constant_id = 1) const uint nMaterials = 1;

layout(location=0) in vec3 iPos;
layout(location=1) in vec2 iTexCoords;
layout(location=2) in vec3 iNormal;
layout(location=3) in vec3 iTangent;
layout(location=4) in vec3 iBitangent;

layout(location=0) out vec4 oColor;

#define LIGHT_UNDEFINED     0x0
#define LIGHT_DIRECTIONAL   0x1
#define LIGHT_POINT         0x2
#define LIGHT_SPOT          0x3
#define LIGHT_AMBIENT       0x4
struct lightSource
{
    vec3 position;
    vec3 direction;
    vec3 attenuation;
    vec3 color;
    float inner_cone;
    float outer_cone;
    uint type;
};
struct packed_lightSource
{
    float position_x;
    float position_y;
    float position_z;
    float direction_x;
    float direction_y;
    float direction_z;
    float attenuation_c;
    float attenuation_l;
    float attenuation_q;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
    float inner_cone;
    float outer_cone;
    uint type;
};
lightSource from_packed(in packed_lightSource P)
{
    return lightSource(
    vec3(
        P.position_x,
        P.position_y,
        P.position_z),
    vec3(
        P.direction_x,
        P.direction_y,
        P.direction_z),
    vec3(
        P.attenuation_c,
        P.attenuation_l,
        P.attenuation_q),
    vec3(
        P.color_r,
        P.color_g,
        P.color_b),
        P.inner_cone,
        P.outer_cone,
        P.type
    );
}

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

layout(std140, binding = 2) uniform uLights
{
    packed_lightSource uLightSources[nLights];
};


layout(binding=0) uniform sampler2D Albedo[nMaterials];

vec3 compute_pointLight(in lightSource L, in vec3 normal, in vec3 pos, in vec3 albedo)
{
    vec3 dir = normalize(L.position - pos);
    float dist = distance(L.position, pos);
    float diff = max(0.0, dot(normal, dir));
    float attenuation = 1.0 / (L.attenuation.x + L.attenuation.y*dist + L.attenuation.z*dist*dist);   
    return albedo*L.color*diff*attenuation;
}
vec3 compute_dirLight(in lightSource L, in vec3 normal, in vec3 albedo)
{
    float diff = max(0.0, dot(normal, L.direction));
    return albedo*L.color*diff;
}

void main()
{
    vec3 color = vec3(0.0);
    vec3 albedo = texture(Albedo[0], iTexCoords).rgb;

    for(int i = 0; i < nLights; i++)
    {
        lightSource L = from_packed(uLightSources[i]);
        switch(L.type){
            case LIGHT_POINT:
                color += compute_pointLight(L, iNormal, iPos, albedo);
                break;
            case LIGHT_DIRECTIONAL:
                color += compute_dirLight(L, iNormal, albedo);
                break;
        }

    }

    oColor = vec4(albedo, 1.0);
}