#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

#include "packed_light.glsl"
#include "BRDF.glsl"
#include "uniform_blocks.glsl"

#ifndef MSAA_LEVEL
#define MSAA_LEVEL 0
#endif

#if MSAA_LEVEL == 0
#define SAMPLERTYPE sampler2D
#else
#define SAMPLERTYPE sampler2DMS
#endif

layout(binding=0) uniform SAMPLERTYPE uAlbedo;
layout(binding=1) uniform SAMPLERTYPE uNormal;
layout(binding=2) uniform SAMPLERTYPE uPosition;
layout(binding=3) uniform SAMPLERTYPE uAO_Rough_Metal;
layout(binding=4) uniform SAMPLERTYPE uEmissive;
layout(binding=5) uniform SAMPLERTYPE uDepth;
layout(binding=6) uniform samplerCube uSkybox;

float luminance(in vec3 c)
{
        return dot(c, vec3(0.299, 0.587, 0.114));
}

float LinearizeDepth(float depth) 
{
    return (2.0 * u_CamNear * u_CamFar) / (u_CamFar + u_CamNear - (depth * 2.0 - 1.0) * (u_CamFar - u_CamNear));	
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
vec3 compute_ambientLight(vec3 V, vec3 H, vec3 N, vec3 albedo, vec3 position, float AO)
{
    vec3 reflected = reflect(V, N);
    return albedo*AO*texture(uSkybox, reflected).rgb*(0.5+0.5*dot(N,vec3(0,1,0)));
}

#if MSAA_LEVEL == 0
vec4 compute_texel()
{
    vec4 out_color = vec4(0);

    vec3 albedo   = texture(uAlbedo, iTexCoords).rgb;
    vec3 normal   = texture(uNormal, iTexCoords).rgb;
    vec3 position = texture(uPosition, iTexCoords).rgb;
    vec3 emissive = texture(uEmissive, iTexCoords).rgb;
    vec3 ARM = texture(uAO_Rough_Metal, iTexCoords).rgb;
    float depth = texture(uDepth, iTexCoords).r;
    float AO = ARM.x;
    float roughness = ARM.y;
    float metallic = 1.0 - ARM.z;

    if (isinf(position.x)){
        return vec4(1.1*emissive, 1.0);
    }

    vec3 V = normalize(position - u_CamPos.xyz);
    vec3 H = normalize(V + normal);

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
                color += AO*compute_ambientLight(L);
                break;
        }
        color += BRDF(H, V, normal, Ldir, roughness, metallic, albedo, radiance);
    }
    //vec3 ambient = BRDF(H, V, normal, reflected, roughness, metallic, albedo, AO*6*texture(uSkybox, reflected).rgb);
    vec3 ambient = compute_ambientLight(H,V,normal, albedo, position, AO);
    out_color = vec4(emissive + ambient + color, 1.0);
    float cd = 0.025*LinearizeDepth(depth);
    float fog = exp2(-cd*cd);
    out_color.rgb = mix(texture(uSkybox, V).rgb, out_color.rgb, fog);
    out_color.a = 1.0;
    return out_color;
}
#else
vec4 compute_texel(int Sample)
{
    vec4 out_color;

    vec3 albedo   = texelFetch(uAlbedo,    ivec2(gl_FragCoord.xy), Sample).rgb;
    vec3 normal   = texelFetch(uNormal,    ivec2(gl_FragCoord.xy), Sample).rgb;
    vec3 position = texelFetch(uPosition,  ivec2(gl_FragCoord.xy), Sample).rgb;
    vec3 emissive = texelFetch(uEmissive,  ivec2(gl_FragCoord.xy), Sample).rgb;
    vec3 ARM = texelFetch(uAO_Rough_Metal, ivec2(gl_FragCoord.xy), Sample).rgb;
    float depth = texelFetch(uDepth, ivec2(gl_FragCoord.xy), Sample).r;
    float AO = ARM.x;
    float roughness = ARM.y;
    float metallic = 1.0 - ARM.z;

    if (any(isinf(position))){
        return vec4(1.1*emissive, 1.0);
    }

    vec3 V = normalize(position - u_CamPos.xyz);
    vec3 H = normalize(V + normal);

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
                color += AO*compute_ambientLight(L);
                break;
        }
        color += BRDF(H, V, normal, Ldir, roughness, metallic, albedo, radiance);
    }
    //vec3 ambient = BRDF(H, V, normal, reflected, roughness, metallic, albedo, AO*6*texture(uSkybox, reflected).rgb);
    vec3 ambient = compute_ambientLight(H,V,normal, albedo, position, AO);
    out_color = vec4(emissive + ambient + color, 1.0);
    float cd = 0.025*LinearizeDepth(depth);
    float fog = exp2(-cd*cd);
    out_color.rgb = mix(texture(uSkybox, V).rgb, out_color.rgb, fog);
    return out_color;
}
#endif

void main()
{
#if MSAA_LEVEL == 0
    oColor = compute_texel();
#else
    for(int n = 0; n < MSAA_LEVEL; n++){
        oColor += compute_texel(n);
    }
    oColor /= MSAA_LEVEL;
#endif
}
