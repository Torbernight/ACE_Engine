#include <GL/glew.h>
#include "shader.hpp"

void Shader::loadFile(const std::filesystem::path& P)
{
        std::ifstream input(P.c_str(), std::ios_base::ate);
        size_t len = input.tellg();
        input.seekg(std::ios_base::beg);
        size_t padding = len%4 == 0 ? 0 : 4 - len%4;
        char* data = new char[len+padding];
        input.read(data, len);
        input.close();
        len += padding;
        m.bin = std::wstring((wchar_t*)data, len/4);
        delete[] data;
}



void Shader::loadBinary(const std::filesystem::path& P)
{
        loadFile(P);
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)m.bin.c_str() , m.bin.length()*4UL);
}


void Shader::loadBinary()
{
        glShaderBinary(1, &m.ShaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)m.bin.c_str() , m.bin.length()*4UL);
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
                int size;
                glGetProgramInfoLog(m.ProgramID, 0, &size, nullptr);
                char *log = new char[size];
                glGetProgramInfoLog(m.ProgramID, size, nullptr, log);
                std::cerr << "Error: program not linked!\n" << log << std::endl;
                delete[] log;
        }
}
