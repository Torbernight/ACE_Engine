#pragma once

#include <utility>

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../framebuffer.hpp"
#include "../shader.hpp"
#include "../texture.hpp"

class canvas_renderer
{
        public:

        void bind();
        void bind(shaderProgram &S);
        void unbind();

        void bind_framebuffer()
        {
                m.FB.bind();
        }
        static void unbind_framebuffer()
        {
                framebuffer::unbind();
        } 
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
        void set_viewport();

        void render();

        void initPostProc(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h);
        void initDeferred(const std::filesystem::path& vShader, const std::filesystem::path& fShader, unsigned int nMaterials, int w, int h);

        void resize(int w, int h)
        { 
                m.w = w;
                m.h = h;
                m.FB.resize(m.w,m.h); 
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

        private:

        struct members
        {
                framebuffer FB;
                Shader vShader;
                Shader fShader;
                shaderProgram sProgram;
                vertexArray VAO;
                buffer VBO;              
                int w, h;

                members()
                : vShader(Vertex), fShader(Fragment), VAO(2), VBO(0x8892) {}
        } m;

        void initVAO();
        void initFBO(int w, int h, std::initializer_list<texture::texture_parameters>&& Attachments);
};