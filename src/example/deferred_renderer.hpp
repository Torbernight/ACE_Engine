#pragma once
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

        void initPostProc(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h);

        void init(const std::filesystem::path& vShader, const std::filesystem::path& fShader, unsigned int nMaterials, int w, int h);

        void resize(int w, int h){ 
                m.FB.resize(w,h); 
        }

        private:

        struct members{
                framebuffer FB;
                Shader vShader;
                Shader fShader;
                shaderProgram sProgram;
                vertexArray VAO;
                buffer VBO;              


                members()
                : vShader(Vertex), fShader(Fragment), VAO(2), VBO(0x8892) {}
        } m;

};