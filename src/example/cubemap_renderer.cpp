#include <GL/glew.h>
#include "../texture_load.hpp"
#include "cubemap_renderer.hpp"

void cubemap_renderer::initVAO()
{
        const float vertices[108] = {
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f
        };

        m.VAO.generateVAO();
        m.VAO.Attributes[0] = {0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0};
        m.VBO.generate();
        m.VAO.bind();
        m.VBO.bufferData(sizeof(vertices), vertices, GL_STATIC_DRAW);
        m.VAO.setVertexAttributes(m.VBO);
        m.VAO.unbind();
}

#define CUBE_RIGHT_FACE  0
#define CUBE_LEFT_FACE   1
#define CUBE_TOP_FACE    2
#define CUBE_BOTTOM_FACE 3
#define CUBE_BACK_FACE   4
#define CUBE_FRONT_FACE  5

//#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515

void cubemap_renderer::init(const std::filesystem::path& vShader, const std::filesystem::path& fShader, int w, int h, int cubemap_binding)
{
    m.vShader.loadBinary(vShader);
    m.fShader.loadBinary(fShader);
    m.vShader.specialize();
    m.sProgram.attachShader(m.vShader);
    m.fShader.add_specialization_const(cubemap_binding, 0);
    m.fShader.specialize();
    m.sProgram.attachShader(m.fShader);
    m.sProgram.linkProgram();

    initVAO();

    image_data faces[6];
    const char* filenames[6] = {
        "ps_right.png",
        "ps_left.png",
        "ps_top.png",
        "ps_bottom.png",
        "ps_back.png",
        "ps_front.png"
    };

    m.cubemap.generate();

    for(int n = 0; n < 6; n++)
    {
        faces[n] = process_image_data(load_image(std::filesystem::path("assets/textures/skybox") / filenames[n]));

        if (0 == n)
        {
            m.cubemap.setWidth(w);
            m.cubemap.setHeight(h);
            m.cubemap.setSlot(cubemap_binding);
            m.cubemap.setTarget(GL_TEXTURE_CUBE_MAP);
            m.cubemap.setType(faces[0].type);
            m.cubemap.setFormat(faces[0].format, faces[0].internal_format);
            m.cubemap.bind();
        }

        m.cubemap.genTexImage_cubemap(n, faces[n].data);

        delete[] faces[n].data;
    }

    m.cubemap.setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m.cubemap.setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m.cubemap.setTexParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m.cubemap.setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m.cubemap.setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m.cubemap.unbind();

}

void cubemap_renderer::render()
{
    m.VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m.VAO.unbind();
}