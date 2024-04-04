
layout(std140, binding=0) uniform Transforms
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
};

layout(std140, binding=1) uniform Camera
{
    vec4 u_CamPos;
    vec4 u_CamFront;
};

layout(std140, binding = 3) uniform Materials
{
    int albedo_index;
    int normal_index;
    int roughness_metallic_index;
};

layout(binding=4, std140) uniform postProcess
{
        float uBrightness;
        float uContrast;
        float uSaturation;
        float uGamma;
        float uExposure;
};
