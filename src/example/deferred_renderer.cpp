#include <GL/glew.h>
#include "deferred_renderer.hpp"

void deferred_renderer::bind()
{
        m.FB.bind();
        m.sProgram.use();
}

void deferred_renderer::unbind()
{
        m.FB.unbind();
        m.sProgram.unuse();
}

void deferred_renderer::render()
{
        m.FB.bindTextures();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT);
        m.VAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m.VAO.unbind();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
}

void deferred_renderer::initPostProc(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h)
{
        m.vShader.loadBinary(vShader);
        m.fShader.loadBinary(fShader);
        m.vShader.specialize();
        m.sProgram.attachShader(m.vShader);
        m.fShader.specialize();
        m.sProgram.attachShader(m.fShader);
        m.sProgram.linkProgram();
        const float vertices[24] = 
        {
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f
        };
        m.VAO.generateVAO();
        m.VAO.Attributes[0] = {0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0};
        m.VAO.Attributes[1] = {1, 2, GL_FLOAT, GL_TRUE,  4*sizeof(float), 2*sizeof(float)};
        m.VBO.generate();
        m.VAO.bind();
        m.VBO.bufferData(24*sizeof(float), vertices, GL_STATIC_DRAW);
        m.VAO.setVertexAttributes(m.VBO);
        m.VAO.unbind();

        m.FB.resize(w, h);
        m.FB.gen_FBO();
        m.FB.bind();
        m.FB.gen_color_attachment({GL_TEXTURE_2D, 0, GL_RGBA, GL_RGB32F, GL_FLOAT, m.FB.getWidth(), m.FB.getHeight()});
        m.FB.setup();

}

void deferred_renderer::init(const std::filesystem::path& vShader, const std::filesystem::path& fShader, unsigned int nMaterials, int w, int h)
{
        m.vShader.loadBinary(vShader);
        m.fShader.loadBinary(fShader);

        m.vShader.specialize();
        m.sProgram.attachShader(m.vShader);
        m.fShader.add_specialization_const(nMaterials, 0);
        m.fShader.specialize();
        m.sProgram.attachShader(m.fShader);
        m.sProgram.linkProgram();

        const float vertices[24] = 
        {
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f
        };
        m.VAO.generateVAO();
        m.VAO.Attributes[0] = {0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0};
        m.VAO.Attributes[1] = {1, 2, GL_FLOAT, GL_TRUE,  4*sizeof(float), 2*sizeof(float)};
        m.VBO.generate();
        m.VAO.bind();
        m.VBO.bufferData(24*sizeof(float), vertices, GL_STATIC_DRAW);
        m.VAO.setVertexAttributes(m.VBO);
        m.VAO.unbind();

        m.FB.resize(w, h);
        m.FB.gen_FBO();
        m.FB.bind();
        m.FB.gen_color_attachment({GL_TEXTURE_2D, 0, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, m.FB.getWidth(), m.FB.getHeight()}); //albedo
        m.FB.gen_color_attachment({GL_TEXTURE_2D, 1, GL_RGB, GL_RGB32F, GL_FLOAT,         m.FB.getWidth(), m.FB.getHeight()}); //normal
        m.FB.gen_color_attachment({GL_TEXTURE_2D, 2, GL_RGB, GL_RGB32F, GL_FLOAT,         m.FB.getWidth(), m.FB.getHeight()}); //pos
        //m.FB.gen_depth_attachment({GL_TEXTURE_2D, 3, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, m.FB.getWidth(), m.FB.getHeight()});

        m.FB.setup();
}

