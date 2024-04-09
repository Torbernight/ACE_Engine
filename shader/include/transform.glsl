layout(constant_id = 0) const uint nModelMatrices = 1;

layout(std140, binding=0) uniform Transforms
{
    mat4 uProjection;
    mat4 uView;
    mat4 uModel[nModelMatrices];
};