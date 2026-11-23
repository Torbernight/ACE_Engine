#pragma once
#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include "shader.hpp"

namespace Renderer 
{
class Shader_Includer : public glslang::TShader::Includer
{

        static
        char* loadFile(const std::filesystem::path& P, size_t& len)
        {
                std::ifstream input(P.c_str(), std::ios_base::ate);
                if (input.fail()){
                        std::cerr << "Error: file " << P << "not found!" << std::endl;
                        assert(false);
                }
                len = input.tellg();
                input.seekg(std::ios_base::beg);
                char *data = new char[len];
                input.read(data, len);
                input.close();
                return data;
        }

        IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) 
        { 
            std::string headerPath;
            size_t size;
            char* data;
            bool found = false;
            std::filesystem::path Include_Dir = "shader/include";

            for(const auto &H : std::filesystem::directory_iterator(Include_Dir)){
                const auto &P = H.path();
                if (P.filename()== headerName){
                    found = true;
                    headerPath = P.c_str();
                    data = loadFile(headerPath, size);
                }
                if(found)
                    break;
            }

            return found ? new IncludeResult(headerPath, data, size, nullptr) : nullptr;
        }

        IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) 
        { 
            std::string headerPath;
            size_t size;
            char* data;
            bool found = false;

            if (strlen(includerName) == 0)
                return nullptr;

            std::filesystem::path ParentPath = std::filesystem::path(includerName).parent_path();

            for(const auto &H : std::filesystem::directory_iterator(ParentPath)){
                const auto &P = H.path();
                if (P.filename()== headerName){
                    found = true;
                    headerPath = P.c_str();
                    data = loadFile(headerPath, size);
                }
                if(found)
                    break;
            }

            return found ? new IncludeResult(headerPath, data, size, nullptr) : nullptr;
        }

        void releaseInclude(IncludeResult* result)
        {
            delete result;
        }
};

struct shader_source
{
    EShLanguage stage;
    std::string source;

    shader_source() = default;

    shader_source(EShLanguage Stage, const std::filesystem::path& P)
    : stage(Stage)
    {
        loadFile(P);
    }

    shader_source(EShLanguage Stage, const std::filesystem::path& P, const std::string& prefix)
    : stage(Stage)
    {
        loadFile(P);
        if (prefix.size() > 0)
        {
            auto first_newline = source.find_first_of('\n'); // find the position of the first newline (just after the version directive)
            source.insert(source.begin()+first_newline+1, prefix.begin(), prefix.end()); // insert prefix after the version directive
        }
    }


    void loadFile(const std::filesystem::path& P)
    {
            std::ifstream input(P.c_str(), std::ios_base::ate);
            if (input.fail()){
                    std::cerr << "Error: file " << P << "not found!" << std::endl;
            }
            size_t len = input.tellg();
            input.seekg(std::ios_base::beg);
            char *data = new char[len];
            input.read(data, len);
            input.close();
            source = std::string(data, len);
            delete[] data;
    }
};

inline static
ShaderStage getStage(EShLanguage lang)
{
    switch(lang)
    {
        case EShLangVertex:
            return ShaderStage_Vertex;
        case EShLangFragment:
            return ShaderStage_Fragment;
        case EShLangCompute:
            return ShaderStage_Compute;
        case EShLangGeometry:
            return ShaderStage_Geometry;
        case EShLangTessEvaluation:
            return ShaderStage_Tess_Evaluation;
        case EShLangTessControl:
            return ShaderStage_Tess_Control;
        default:
            std::cerr << "----------\nWARNING: unidentified EShLanguage value\n----------\n";
            return ShaderStage_Vertex;
    }
}


inline static
Shader compile_shader(const shader_source& src, const char* preprocessor_const = nullptr)
{
    Shader_Includer Includer;
    glslang::InitializeProcess();
    glslang::TShader S(src.stage);
    std::string Ssource = src.source;
    if (preprocessor_const){
        std::size_t I = Ssource.find("#version 460");
        Ssource.insert(I+13, std::string(preprocessor_const));
    }
    const char* source = Ssource.data();
    S.setStrings(&source, 1);
    S.setEnvInput(glslang::EShSource::EShSourceGlsl, src.stage, glslang::EShClient::EShClientOpenGL, 460);
    S.setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EShTargetClientVersion::EShTargetOpenGL_450);
    S.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);
    if (!S.parse(GetDefaultResources(), 460, EProfile::ECoreProfile, true, true, EShMessages::EShMsgDebugInfo, Includer))
    {
        std::cerr << "\n" << S.getInfoLog() << "\n" << S.getInfoDebugLog() << std::endl;

    }
    std::vector<unsigned int> SPIRV;
    glslang::GlslangToSpv(*S.getIntermediate(), SPIRV);
    glslang::FinalizeProcess();
    return Shader(getStage(src.stage), SPIRV);
}

inline static
void compile_shader_SPV(const shader_source& src, std::vector<unsigned int>& SPIRV, const char* preprocessor_const = nullptr)
{
    Shader_Includer Includer;
    glslang::InitializeProcess();
    glslang::TShader S(src.stage);
    std::string Ssource = src.source;
    if (preprocessor_const){
        std::size_t I = Ssource.find_first_of('\n');
        Ssource.insert(I+1, std::string(preprocessor_const));
    }
    const char* source = Ssource.data();
    S.setStrings(&source, 1);
    S.setEnvInput(glslang::EShSource::EShSourceGlsl, src.stage, glslang::EShClient::EShClientOpenGL, 460);
    S.setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EShTargetClientVersion::EShTargetOpenGL_450);
    S.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);
    if (!S.parse(GetDefaultResources(), 460, EProfile::ECoreProfile, true, true, EShMessages::EShMsgDebugInfo, Includer))
    {
        std::cerr << "\n" << S.getInfoLog() << "\n" << S.getInfoDebugLog() << std::endl;
    }
    glslang::GlslangToSpv(*S.getIntermediate(), SPIRV);
    glslang::FinalizeProcess();
}

}