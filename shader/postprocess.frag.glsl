#version 460

layout(location=0) in vec2 iTexCoords;
layout(location=0) out vec4 oColor;

layout(binding=0) uniform sampler2D uScreen;

layout(binding=4, std140) uniform postProcess
{
        float uBrightness;
        float uContrast;
        float uSaturation;
        float uGamma;
};

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

void main()
{
        vec2 resolution = textureSize(uScreen, 0).xy;
        vec2 texcoords = floor(iTexCoords*resolution*0.25)*4/resolution;
        vec3 color = texture(uScreen, texcoords).rgb;

        color = applyContrastandBrigtness(color, uContrast, uBrightness);
        color = applySaturation(color, uSaturation);

        color = applyGamma(color, uGamma);

        oColor = vec4(color, 1.0);
}