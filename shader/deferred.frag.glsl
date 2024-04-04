#version 460

#extension GL_GOOGLE_include_directive : enable

layout(constant_id = 0) const uint nLights = 1;

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

#include "packed_light.glsl"
#include "BRDF.glsl"
#include "uniform_blocks.glsl"

layout(binding=0) uniform sampler2D uAlbedo;
layout(binding=1) uniform sampler2D uNormal;
layout(binding=2) uniform sampler2D uPosition;
layout(binding=3) uniform sampler2D uRoughness_Metallic;
layout(binding=4) uniform sampler2D uDepth;

float LinearizeDepth(float depth) 
{
    const float near = 0.1;
    const float far = 1000.0;
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

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
vec3 compute_spotLight(in lightSource L, in vec3 normal, in vec3 pos, in vec3 albedo)
{
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
    vec2 RM = texture(uRoughness_Metallic, iTexCoords).rg;
    float roughness = RM.x;
    float metallic = 1.0 - RM.y;

    vec3 color = vec3(0.0);
    for(int i = 0; i < nLights; i++)
    {
        lightSource L = from_packed(uLightSources[i]);
        vec3 radiance = vec3(0);
        vec3 Ldir = vec3(0);
        switch(L.type)
        {
            case LIGHT_POINT:
                radiance = compute_pointLight(L, normal, position, albedo);
                Ldir = normalize(L.position - position);
                break;
            case LIGHT_DIRECTIONAL:
                radiance = compute_dirLight(L, normal, albedo);
                Ldir = L.direction;
                break;
            case LIGHT_SPOT:
                radiance = compute_spotLight(L, normal, position, albedo);
                Ldir = normalize(L.position - position);
                break;
            case LIGHT_AMBIENT:
                color += albedo*compute_ambientLight(L);
                break;
        }
        vec3 V = normalize(position - u_CamPos.xyz);
        vec3 H = normalize(V + normal);
        color += BRDF(H, V, normal, Ldir, roughness, metallic, albedo, radiance);
    }
    oColor = vec4(color, 1.0);
    
    float cd = 0.025*LinearizeDepth(texture(uDepth, iTexCoords).r);
    float fog = exp2(-cd*cd);
    oColor.rgb = mix(vec3(0), oColor.rgb, fog);
}