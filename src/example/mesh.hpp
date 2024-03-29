#pragma once

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#define GL_FLOAT 0x1406
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11


class mesh
{
    public:
    struct vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
    
    vertexArray VAO;
    buffer VBO, EBO;

    mesh()
    : 
    VAO({
        {0, 3 , GL_FLOAT, 0, sizeof(vertex), 0},
        {1, 2 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, texcoord)},
        {2, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, normal)},
        {3, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, tangent)},
        {4, 3 , GL_FLOAT, 1, sizeof(vertex), offsetof(vertex, bitangent)}
        }), 
    
    VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER)
    {}

    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    glm::mat4 transform;
    unsigned int materialIndex = 0;

    void init();
    void render();

};
