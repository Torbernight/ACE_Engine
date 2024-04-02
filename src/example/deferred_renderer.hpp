#pragma once

#include <utility>

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "../framebuffer.hpp"
#include "../shader.hpp"
#include "../texture.hpp"

class scene;

class deferred_renderer
{
        friend class scene;
        
        public:

        void bind();
        void unbind();

        void render();
        void render(int w, int h);

        void initPostProc(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h);

        void init(const std::filesystem::path& vShader, const std::filesystem::path& fShader, unsigned int nMaterials, int w, int h);

        void resize(int w, int h){ 
                m.w = w;
                m.h = h;
                m.FB.resize(m.w,m.h); 
        }

        private:

        struct members{
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