
layout(std140, binding = 3) uniform Materials
{
    int   uAlbedo_index;
    int   uNormal_index;
    int   uAO_roughness_metallic_index;
    int   uEmissive_index;
    int   uGlow;
    float uAlpha;
};

layout(std140, binding = 6) uniform Sprites
{
    int uSpriteIndex;
    int uFPS;
};