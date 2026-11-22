
#ifndef NLIGHTS
#define NLIGHTS 1
#endif

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
    float inner_cone;
    float outer_cone;
    uint type;
    int shadow;
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

layout(std140, binding = 2) uniform uLights
{
    packed_lightSource uLightSources[NLIGHTS];
};