
#extension GL_ARB_gpu_shader_int64 : enable

layout(std140, binding=1) uniform Camera
{
    vec4 u_CamPos;
    vec4 u_CamFront;
    float u_CamNear;
    float u_CamFar;
};

layout(binding=4, std140) uniform postProcess
{
    float uBrightness;
    float uContrast;
    float uSaturation;
    float uGamma;
    float uExposure;
};

layout(binding=5, std140) uniform GameState
{
    uint64_t uFrame;
    float uTime;
    float uDelta;
    vec2 resolution;
};