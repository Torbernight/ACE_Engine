#pragma once

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "AABB.hpp"
#include "frustum.hpp"
#include <cstring>
#include <string>
#include <regex>
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

        bool operator==(const vertex& V) const
        {
            return
            position == V.position &&
            texcoord == V.texcoord &&
            normal == V.normal &&
            tangent == V.tangent &&
            bitangent == V.bitangent;
        }
    };
    
    std::string Name;
    vertexArray VAO;
    buffer VBO;
    buffer EBO;
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    AABB bounding_box;
    glm::mat4 lastFrame_Transform = glm::mat4(1.0f);
    std::vector<glm::mat4> transforms;
    unsigned int nInstances = 1;
    unsigned int nVisibleInstances = 1;
    unsigned int materialIndex = 0;
    bool discard = false;
    bool collision = true;
    bool static_mesh = true;
    bool grabbable = false;
    bool selected = false;
    bool grabbed = false;

    glm::vec3 acc =   glm::vec3(0.0f);
    glm::vec3 vel = glm::vec3(0.0f);

    void apply_transform();
    void join_identical_vertices();

    void compute_AABB();

    void onFrustum(const frustum& F);
    bool isInstance(const mesh& M) const;

    bool operator==(const mesh& M) const 
    {
        return isInstance(M); 
    }

    mesh()
    : 
    VAO({
        {0, 3 , GL_FLOAT, 0, sizeof(vertex), 0},
        {1, 2 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, texcoord)},
        {2, 3 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, normal)},
        {3, 3 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, tangent)},
        {4, 3 , GL_FLOAT, 0, sizeof(vertex), offsetof(vertex, bitangent)}
        }), 
    
    VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER)
    {}
    
    void init();
    void render();

};
