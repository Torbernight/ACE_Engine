#pragma once

#include "framebuffer.hpp"
#include "vertexArray.hpp"
#include "buffer.hpp"
#include "shader.hpp"

namespace Renderer 
{
class CubemapRenderer
{
    public:

    void use_shader()
    {
            m.sProgram.use();
    }
    static void unuse_shader()
    {
            shaderProgram::unuse();
    }
    int get_width() const{
            return m.w;
    }
    int get_height() const{
            return m.h;
    }

    void init(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h, const std::filesystem::path& skybox, int cubemap_binding = 6);
    void render();

    void bind_texture()
    {
        m.cubemap.bind_unit();
    }

    Shader* getShader(ShaderStage stage)
    {
            switch(stage)
            {
                    case Vertex:
                            return &m.vShader;
                    case Fragment:
                            return &m.fShader;
                    default:
                        return nullptr;
            }
    }

    operator bool() const
    {
        return m.cubemap;
    }

    private:
    struct members
    {
            Texture cubemap;
            Shader vShader;
            Shader fShader;
            shaderProgram sProgram;
            vertexArray VAO;
            Buffer VBO;              
            int w, h;

            members()
            : vShader(Vertex), fShader(Fragment), VAO(1), VBO(0x8892) {}
    } m;

    void initVAO();

};

}