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
        glClear(GL_COLOR_BUFFER_BIT);
        m.VAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m.VAO.unbind();
        glEnable(GL_DEPTH_TEST);
}

void deferred_renderer::render(int w, int h)
{
        glViewport(0,0,w,h);
        m.FB.bindTextures();
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        m.VAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m.VAO.unbind();
        glEnable(GL_DEPTH_TEST);
}

void deferred_renderer::initVAO()
{
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

        initVAO();

        initFBO(w,h, {
        {GL_TEXTURE_2D, 0, GL_RGB, GL_RGB16F, GL_FLOAT, (unsigned int)w, (unsigned int)h}
        });
}

void deferred_renderer::initFBO(int w, int h, std::initializer_list<texture::texture_parameters>&& Attachments)
{
        m.w = w;
        m.h = h;
        m.FB.resize(w, h);
        m.FB.gen_FBO();
        m.FB.bind();
        for (auto A: Attachments)
        {
                if (A.format == GL_DEPTH_COMPONENT || A.format == GL_DEPTH_STENCIL)
                        m.FB.gen_depth_attachment(A);
                else
                        m.FB.gen_color_attachment(A);
        }
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

        initVAO();

        initFBO(w, h, 
        {
        {GL_TEXTURE_2D, 0, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, (unsigned int)w, (unsigned int)h}, //albedo
        {GL_TEXTURE_2D, 1, GL_RGB, GL_RGB32F, GL_FLOAT,         (unsigned int)w, (unsigned int)h}, //normal
        {GL_TEXTURE_2D, 2, GL_RGB, GL_RGB32F, GL_FLOAT,         (unsigned int)w, (unsigned int)h}, //pos
        {GL_TEXTURE_2D, 3, GL_RGB, GL_RGB16F, GL_FLOAT,         (unsigned int)w, (unsigned int)h}, //AO_roughness_metallic
        {GL_TEXTURE_2D, 4, GL_RGB, GL_RGB16F, GL_FLOAT,         (unsigned int)w, (unsigned int)h}, //emissive
        {GL_TEXTURE_2D, 5, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8, (unsigned int)w, (unsigned int)h}
        });

}

