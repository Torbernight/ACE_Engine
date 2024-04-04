#version 460

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

layout(binding=0) uniform sampler2D uScreen;

#include "uniform_blocks.glsl"

float luminance(in vec3 c)
{
        return dot(c, vec3(0.299, 0.587, 0.114));
}

vec3 grayscale(in vec3 c)
{
        return vec3(luminance(c));
}

vec3 applyGamma(in vec3 c, in float gamma)
{
        return pow(c, vec3(gamma));
}

vec3 applyContrastandBrigtness(in vec3 c, in float contrast, in float brightness)
{
        return clamp(contrast*(c-0.5)+0.5+brightness,vec3(0.0),vec3(1.0));
}

vec3 applySaturation(in vec3 c, in float saturation)
{
        return clamp(mix(grayscale(c), c, saturation),vec3(0.0),vec3(1.0));
}

vec3 blackBars(in vec2 uv, in vec3 col, in float oldAspect, in float newAspect)
{
        vec2 uv2 = (uv*2-1)*vec2(oldAspect, 1.0);
        return abs(uv2.x) > newAspect ? vec3(0.0) : col;
}

vec3 change_luminance(vec3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}

vec3 reinhard_extended(vec3 v, float max_white)
{
    return v * (1.0 + (v / max_white * max_white)) / (1.0 + v);
}

vec3 reinhard_extended_luminance(in vec3 v, in float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.0 + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0 + l_old);
    return change_luminance(v, l_new);
}

vec4 blerp(vec2 texcoords, vec2 resolution, sampler2D tex)
{
        vec2 tc = texcoords*resolution/2;
        vec2 a00 = 2*floor(tc)/resolution;
        vec2 a11 = 2*ceil(tc)/resolution;
        vec2 a01 = vec2(a00.x, a11.y);
        vec2 a10 = vec2(a11.x, a00.y);
        vec2 fr = fract(tc);

        return mix(
                mix(texture(tex, a00),texture(tex, a10),fr.x),
                mix(texture(tex, a01),texture(tex, a11),fr.x),fr.y);
}

void main()
{
        vec2 resolution = textureSize(uScreen, 0).xy;
        vec3 color = texture(uScreen, iTexCoords).rgb;

        color = clamp(reinhard_extended_luminance(color, uExposure), vec3(0),vec3(1));
        color = applyContrastandBrigtness(color, uContrast, uBrightness);
        color = applySaturation(color, uSaturation);

        color = applyGamma(color, uGamma);

        color = blackBars(iTexCoords, color, resolution.x/resolution.y, 4.0/3.0);

        oColor = vec4(color, 1.0);
}