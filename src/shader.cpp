#include <utility>
#include <GL/glew.h>
#include "shader.hpp"

template<ShaderStage Stage>
void Shader<Stage>::loadBinary(const std::filesystem::path&)
{
        // ... load file here
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)m.bin.c_str() , m.bin.length()*4UL);
}

template<ShaderStage Stage>
void Shader<Stage>::loadBinary()
{
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)m.bin.c_str() , m.bin.length()*4UL);
}


template<ShaderStage Stage>
Shader<Stage>::Shader()
{
        m.ShaderID = glCreateShader(Stage);
}
template<ShaderStage Stage>
Shader<Stage>::Shader(Shader& S)
: m(S.m)
{
        S.m = {};
}
template<ShaderStage Stage>
Shader<Stage>::Shader(Shader&& S)
: m(std::move(S.m))
{
}

template<ShaderStage Stage>
Shader<Stage>& Shader<Stage>::operator=(Shader& S)
{
        Shader<Stage> temp(S);
        this->m = temp.m;
        temp.m = {};
        return *this;
}

template<ShaderStage Stage>
Shader<Stage>& Shader<Stage>::operator=(Shader&& S)
{
        Shader<Stage> temp(S);
        this->m = temp.m;
        temp.m = {};
        return *this;
}

template<ShaderStage Stage>
Shader<Stage>::~Shader()
{
        if (m.ShaderID != 0)
                glDeleteShader(m.ShaderID);
}