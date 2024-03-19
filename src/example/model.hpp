#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "../obj_importer.hpp"
#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "material.hpp"

class Model
{
    public:
    obj Mesh;
    vertexArray VAO;
    buffer VBO;
    buffer EBO;
    glm::mat4 Transform;
    Material red_metal;

    Model();

    void init();

    void render();
};