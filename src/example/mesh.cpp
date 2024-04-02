#include <GL/glew.h>
#include "mesh.hpp"

void mesh::init()
{
    VBO.generate();
    VBO.bufferData(vertices.size()*sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
    VAO.generateVAO();
    VAO.setVertexAttributes(VBO);
    VAO.bind();
    EBO.generate();
    EBO.bind();
    EBO.bufferData(indices.size()*4UL, indices.data(), GL_STATIC_DRAW);
    VAO.unbind();
}

void mesh::render()
{
    VAO.bind();
    EBO.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    VAO.unbind();
}