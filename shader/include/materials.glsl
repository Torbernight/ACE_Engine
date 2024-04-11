
layout(std140, binding = 3) uniform Materials
{
    int albedo_index;
    int normal_index;
    int roughness_metallic_index;
    int emissive_index;
};