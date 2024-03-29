#version 460

layout(constant_id = 0) const uint nLights = 1;

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

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

layout(binding=0) uniform sampler2D uAlbedo;
layout(binding=1) uniform sampler2D uNormal;
layout(binding=2) uniform sampler2D uPosition;
layout(binding=3) uniform sampler2D uDepth;

#define M_2_PI 1.5707963267948965580
#define M_PI  3.1415926535897931160
#define M_PI2 6.2831853071795862320

float candela_to_watt(in float x)
{
    return x * (4*M_PI/683.0);
}
vec3 normalize_color(in vec3 c)
{
    return clamp(c, vec3(0), vec3(1));
}

vec3 compute_pointLight(in lightSource L, in vec3 normal, in vec3 pos, in vec3 albedo)
{
    //vec3 color = clamp(L.color, vec3(0.0), vec3(1.0));
    vec3 color = normalize_color(L.color);
    vec3 dir = normalize(L.position - pos);
    float dist = distance(L.position, pos);
    float diff = max(0.0, dot(normal, dir));
    float attenuation = 1.0 / (L.attenuation.x + L.attenuation.y*dist + L.attenuation.z*dist*dist);   
    return albedo*color*diff*attenuation;
}
vec3 compute_dirLight(in lightSource L, in vec3 normal, in vec3 albedo)
{
    //vec3 color = clamp(L.color, vec3(0.0), vec3(1.0));
    vec3 color = normalize_color(L.color);
    float diff = max(0.0, dot(normal, L.direction));
    return albedo*color*diff;
}
vec3 compute_spotLight(in lightSource L, in vec3 normal, in vec3 pos, in vec3 albedo)
{
    //vec3 color = clamp(L.color, vec3(0.0), vec3(1.0));
    vec3 color = normalize_color(L.color);
    vec3 dir = normalize(L.position - pos);
    vec3 ldir = L.direction;
    float dist = distance(L.position, pos);
    float diff = max(0.0, dot(normal, dir));
    float theta = acos(dot(dir,-ldir));
    float spot = 1.0 - smoothstep(L.inner_cone, L.outer_cone, theta);
    float attenuation = spot / (L.attenuation.x + L.attenuation.y*dist + L.attenuation.z*dist*dist);   
    return albedo*L.color*diff;
}
vec3 compute_ambientLight(in lightSource L)
{
    return L.color;
}
void main()
{
    vec3 albedo = texture(uAlbedo, iTexCoords).rgb;
    vec3 normal = texture(uNormal, iTexCoords).rgb;
    vec3 position = texture(uPosition, iTexCoords).rgb;

    vec3 color = vec3(0.0);

    color += albedo*0.1;

    for(int i = 0; i < nLights; i++)
    {
        lightSource L = from_packed(uLightSources[i]);
        switch(L.type){
            case LIGHT_POINT:
                color += compute_pointLight(L, normal, position, albedo);
                break;
            case LIGHT_DIRECTIONAL:
                color += compute_dirLight(L, normal, albedo);
                break;
            case LIGHT_SPOT:
                color += compute_spotLight(L, normal, position, albedo);
                break;
            case LIGHT_AMBIENT:
                color += compute_ambientLight(L);
                break;
        }
    }
    oColor = vec4(color, 1.0);
}