#include <iostream>
#include <glad/gl.h>
#include <cstring>
#include <utility>
#include <fstream>
#include <iostream>
#include "shader.hpp"

namespace Renderer {


void Shader::loadFile(const std::filesystem::path& P)
{
        std::ifstream input(P.c_str(), std::ios_base::ate);
        if (input.fail()){
                std::cerr << "Error: file " << P << "not found!" << std::endl;
        }
        size_t len = input.tellg();
        input.seekg(std::ios_base::beg);
        //size_t padding = len%4 == 0 ? 0 : 4 - len%4;
        m.bin.resize(len/4);
        uint32_t *data = m.bin.data();
        input.read((char*)data, len);
        input.close();
        //len += padding;
        //m.bin = std::wstring((wchar_t*)data, len/4);
        //delete[] data;
}

void Shader::reloadBinary(const std::vector<unsigned int>& bin)
{
        m.bin = bin;
        glDeleteShader(m.ShaderID);
        m.ShaderID = glCreateShader(m.stage);
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, m.bin.data() , m.bin.size()*4UL);
}

void Shader::loadBinary(const std::filesystem::path& P)
{
        loadFile(P);
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, m.bin.data() , m.bin.size()*4UL);
}

Shader::Shader(ShaderStage stage, const std::vector<uint32_t>& bin)
{
        m.stage = stage;
        m.ShaderID = glCreateShader(stage);
        loadEmbedded(bin.data(), bin.size());
}


void Shader::loadEmbedded(const uint32_t* data, size_t len)
{
        m.bin.resize(len);
        std::memcpy(m.bin.data(), data, len*4);
        //m.bin = std::wstring((wchar_t*)data, len);
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, m.bin.data() , m.bin.size()*4UL);
}



void Shader::loadBinary()
{
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, m.bin.data() , m.bin.size()*4UL);
}


Shader::Shader(ShaderStage Stage, const std::filesystem::path& P)
{
        m.stage = Stage;
        m.ShaderID = glCreateShader(Stage);
        loadBinary(P);
}


Shader::Shader(ShaderStage Stage)
{
        m.stage = Stage;
        m.ShaderID = glCreateShader(Stage);
}

Shader::Shader(ShaderStage stage, const uint32_t* bin, size_t size)
{
        m.stage = stage;
        loadEmbedded(bin, size);
}

Shader::Shader(Shader& S)
: m(S.m)
{
        S.m = {};
}

Shader::Shader(Shader&& S)
: m(std::move(S.m))
{
}


Shader& Shader::operator=(Shader& S)
{
        Shader temp(S);
        this->m = temp.m;
        temp.m = {};
        return *this;
}


Shader& Shader::operator=(Shader&& S)
{
        Shader temp(S);
        this->m = temp.m;
        temp.m = {};
        return *this;
}


Shader::~Shader()
{
        if (m.ShaderID != 0)
                glDeleteShader(m.ShaderID);
}


void Shader::add_specialization_const(uint32_t value, uint32_t index)
{
        m.specialization_constants.push_back(value);
        m.specialization_constants_indices.push_back(index);
}


void Shader::specialize()
{
        uint32_t *values = nullptr, *indices = nullptr;
        if (!m.specialization_constants.empty())
        {
                indices = &m.specialization_constants_indices[0];
                values = &m.specialization_constants[0];
        }
        glSpecializeShader(m.ShaderID, "main", m.specialization_constants.size(), indices, values);
        getStatus();
}


void Shader::getStatus()
{
        int status;
        glGetShaderiv(m.ShaderID, GL_COMPILE_STATUS, &status);
        m.status = status == GL_TRUE;
        if (!m.status)
        {
                char *log = new char[512];
                glGetShaderInfoLog(m.ShaderID, 512, nullptr, log);
                std::cerr << "Error: " << get_stage_str() << " not compiled!\n" << log << std::endl;
                delete[] log;
        }
}


shaderProgram::shaderProgram()
{
        m.ProgramID = glCreateProgram();
}
shaderProgram::shaderProgram(shaderProgram&& P)
: m(std::move(P.m))
{
}

shaderProgram& shaderProgram::operator=(shaderProgram&& P)
{
        shaderProgram temp(P);
        return (*this = temp);
}
shaderProgram::shaderProgram(shaderProgram& P)
: m(P.m)
{
        P.m = {};
}
shaderProgram& shaderProgram::operator=(shaderProgram& P)
{
        m = P.m;
        P.m = {};
        return *this;
}
shaderProgram::~shaderProgram()
{
        if (m.ProgramID != 0)
                glDeleteProgram(m.ProgramID);
}

void shaderProgram::use()
{
        glUseProgram(m.ProgramID);
}
void shaderProgram::unuse()
{
        glUseProgram(0);
}


void shaderProgram::attachShader(const Shader& S)
{
        glAttachShader(m.ProgramID, S.getShaderID());
}

void shaderProgram::dettachShader(const Shader& S)
{
        glDetachShader(m.ProgramID, S.getShaderID());
}


void shaderProgram::linkProgram()
{
        glLinkProgram(m.ProgramID);
        get_status();
}

void shaderProgram::get_status()
{
        int status;
        glGetProgramiv(m.ProgramID, GL_LINK_STATUS, &status);
        if (GL_FALSE == status)
        {
                int len;
                glGetProgramiv(m.ProgramID, GL_INFO_LOG_LENGTH, &len);
                char *log = new char[len+1];
                glGetProgramInfoLog(m.ProgramID, len+1, nullptr, log);
                std::cerr << "Error: program not linked!\n" << log << std::endl;
                delete[] log;
        }
}

}