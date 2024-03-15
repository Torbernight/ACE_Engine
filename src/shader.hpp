#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <cstdint>

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

    void loadBinary(const std::filesystem::path&);
    void loadBinary();

    void specialize();

    unsigned int getShaderID() const
    {
        return m.ShaderID;
    }

    Shader();
    Shader(const std::filesystem::path&);
    Shader(Shader&);
    Shader(Shader&&);
    Shader& operator=(Shader&);
    Shader& operator=(Shader&&);
    ~Shader();

    bool isCompiled()
    {
        return m.status;
    }

    void add_specialization_const(uint32_t value, uint32_t index);

    private:
    struct members{
        unsigned int ShaderID = 0;
        std::wstring bin; //intending to only use spir-V shaders, but maybe add compiling in runtime
        std::vector<uint32_t> specialization_constants;
        std::vector<uint32_t> specialization_constants_indices;
        bool status = false;
    } m;

    void loadFile(const std::filesystem::path&);

    void getStatus();

    inline static
    const char* get_stage_str()
    {
        switch(Stage)
        {
                case Vertex:
                        return "Vertex Shader";
                case Fragment:
                        return "Fragment Shader";
                case Tess_Control:
                        return "Tesselation Control Shader";
                case Tess_Evaluation:
                        return "Tesselation Evaluation Shader";
                case Geometry:
                        return "Geometry Shader";
                case Compute:
                        return "Compute Shader";
        }
    }

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

    template<ShaderStage Stage>
    void attachShader(const Shader<Stage>& S);
    void linkProgram();

    private:
    struct members
    {
        unsigned int ProgramID = 0;
    } m;

    void get_status();
};

#include "shader.inl"