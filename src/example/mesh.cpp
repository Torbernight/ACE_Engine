#include <GL/glew.h>
#include <algorithm>
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

void mesh::apply_transform()
{
    using namespace glm;
    glm::mat3 ntransform = glm::transpose(glm::inverse(glm::mat3(this->transform)));
    for(auto &V : vertices)
    {
        V.position = vec3(transform * vec4(V.position, 1.0));
        V.normal = ntransform * V.normal;
        V.tangent = ntransform * V.tangent;
        V.bitangent = ntransform * V.bitangent;
    }
    transform = glm::mat4(1.0f);
}

void mesh::join_identical_vertices()
{
    std::vector<unsigned int> repeated;
    
    for(auto it = vertices.begin(); it < vertices.end(); it++)
    {
        if (0 == std::count(it+1, vertices.end(), *it))
            continue;

        unsigned int index = it - vertices.begin();
        auto res = it;

        while((res = std::find(res+1, vertices.end(), *it)) != vertices.end())
        {
            unsigned int old_index = res - vertices.begin();
            repeated.push_back(old_index);
            std::replace(indices.begin(), indices.end(), old_index, index);
        }
    }
    if (!repeated.empty())
        vertices.erase(std::remove_if(vertices.begin(), vertices.end(), [&](vertex &V){return std::count(repeated.begin(), repeated.end(), (unsigned int)(&V - &vertices[0])) > 0;}));
}

void mesh::render()
{
    VAO.bind();
    EBO.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    VAO.unbind();
}