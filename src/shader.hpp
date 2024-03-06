#pragma once

#include <optional>
#include <string>

/* 
* * * * * * * from glew.h * * * * * * * * 
#define GL_VERTEX_SHADER 0x8B31
#define GL_TESS_CONTROL_SHADER 0x8E88
#define GL_TESS_EVALUATION_SHADER 0x8E87
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPUTE_SHADER 0x91B9
* * * * * * * * * * * * * * * * * * * * * 
*/

enum ShaderStage
{
    Vertex = 0x8B31,
    Tess_Control = 0x8E88,
    Tess_Evaluation = 0x8E87,
    Geometry = 0x8DD9,
    Fragment = 0x8B30,
    Compute = 0x91B9
};

template<ShaderStage Stage>
class Shader
{
    public:


    private:
    struct members{
        std::string bin; //intending to only use spir-V shaders, but maybe add compiling in runtime
    } m;

    inline static constexpr ShaderStage s_stage = Stage; 
};

class shaderProgram
{
    public:

    shaderProgram();
    shaderProgram(shaderProgram&&);
    shaderProgram& operator=(shaderProgram&&);
    shaderProgram(shaderProgram&);
    shaderProgram& operator=(shaderProgram&);
    ~shaderProgram();


    void use();
    static void unuse();

    private:

    struct members
    {
        unsigned int ProgramID;
        std::optional<Shader<Vertex>>           VertexStage;
        std::optional<Shader<Fragment>>         FragmentStage;
        std::optional<Shader<Tess_Control>>     TessControlStage;
        std::optional<Shader<Tess_Evaluation>>  TessEvaluationStage;
        std::optional<Shader<Geometry>>         GeometryStage;
        std::optional<Shader<Compute>>          ComputeStage;
    } m;
};