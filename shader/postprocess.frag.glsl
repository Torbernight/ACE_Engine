#version 460

layout(early_fragment_tests) in;

#extension GL_GOOGLE_include_directive : enable

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

layout(binding=0) uniform sampler2D uScreen;

#include "uniform_blocks.glsl"

float luminance(in vec3 c)
{
        return dot(c, vec3(0.299, 0.587, 0.114));
}
float luminance(in vec4 c)
{
        return dot(c.xyz, vec3(0.299, 0.587, 0.114));
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
vec4 change_luminance(vec4 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}

vec3 reinhard_extended(vec3 v, float max_white)
{
    return v * (1.0 + (v / max_white * max_white)) / (1.0 + v);
}

float reinhard_extended(float v, float max_white)
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
vec4 reinhard_extended_luminance(in vec4 v, in float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.0 + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0 + l_old);
    return change_luminance(v, l_new);
}

#include "fxaa2.glsl"

vec4 apply_FXAA(in vec2 resolution)
{
        return vec4(FxaaPixelShader(iTexCoords, uScreen, 1.0/resolution), 1.0);
}

vec3 crosshair(in vec3 color, in vec2 resolution)
{
        int size = 5;
        int line_width = 1;
        vec2 TC = resolution*abs(iTexCoords*2.0-1.0);
        vec3 col = TC.x <= float(line_width) || TC.y <= float(line_width) ? 1.0 - color : color;
        col = TC.x > float(size) || TC.y > float(size) ? color : col;

        return col;
}

void main()
{
        vec3 color;
        if(uFXAA){
                color = apply_FXAA(uResolution).rgb;
        } else {
                color = clamp(reinhard_extended_luminance(texture(uScreen, iTexCoords).rgb, uExposure), vec3(0),vec3(1));
        }

        color = applyContrastandBrigtness(color, uContrast, uBrightness);
        
        color = applySaturation(color, uSaturation);

        color = applyGamma(color, uGamma);

        color = blackBars(iTexCoords, color, uResolution.x/uResolution.y, 4.0/3.0);

        color = crosshair(color, uResolution);

        oColor = vec4(color, 1.0);
}