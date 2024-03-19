#include <GL/glew.h>
#include "model.hpp"


Model::Model() : VAO(3), VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER), Transform(1.0f) {}

void Model::init()
{
    Mesh.import("assets/teapot.obj");
    Mesh.smooth_normals();
    Mesh.gen_index();
    
    red_metal.load("assets/red_metal.jpg");

    Transform = glm::translate(Transform, glm::vec3(0.0, 0.0, -10.0));
    Transform = glm::scale(Transform, glm::vec3(0.1f));

    VAO.Attributes[0] = {0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0};
    VAO.Attributes[1] = {1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)offsetof(obj::vertex, texcoord)};
    VAO.Attributes[2] = {2, 3, GL_FLOAT, GL_TRUE,  8*sizeof(float), (void*)offsetof(obj::vertex, normal)};
    VBO.generate();
    VBO.bufferData(Mesh.getVertices().size()*sizeof(obj::vertex), Mesh.getVertices().data(), GL_STATIC_DRAW);
    VAO.generateVAO();
    VAO.setVertexAttributes(VBO, 0, sizeof(obj::vertex));
    VAO.bind();
    EBO.generate();
    EBO.bind();
    EBO.bufferData(Mesh.getIndices().size()*4UL, Mesh.getIndices().data(), GL_STATIC_DRAW);
    VAO.unbind();
}

void Model::render()
{
    VAO.bind();
    EBO.bind();
    red_metal.bind();
    glDrawElements(GL_TRIANGLES, Mesh.getIndices().size(), GL_UNSIGNED_INT, NULL);
    VAO.unbind();
}