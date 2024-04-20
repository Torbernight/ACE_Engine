
layout(std140, binding = 3) uniform Materials
{
    int albedo_index;
    int normal_index;
    int AO_roughness_metallic_index;
    int emissive_index;
    int glow;
    float alpha;
};