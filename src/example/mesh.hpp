#pragma once

#include "../vertexArray.hpp"
#include "../buffer.hpp"
#include "geometry/AABB.hpp"
#include "geometry/frustum.hpp"
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


class Mesh
{
    public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        bool operator==(const Vertex& V) const
        {
            return
            position == V.position &&
            texcoord == V.texcoord &&
            normal == V.normal &&
            tangent == V.tangent &&
            bitangent == V.bitangent;
        }
    };
    
    struct members
    {
        std::string name;
        vertexArray VAO;
        Buffer VBO;
        Buffer EBO;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        AABB bounding_box;
        glm::mat4 lastFrameTransform = glm::mat4(1.0f);
        glm::mat4 transform;
        bool visible = true;
        unsigned int materialIndex = 0;
        bool discard = false;

        members():
        VAO({
            {0, 3 , GL_FLOAT, 0, sizeof(Vertex), 0},
            {1, 2 , GL_FLOAT, 0, sizeof(Vertex), offsetof(Vertex, texcoord)},
            {2, 3 , GL_FLOAT, 0, sizeof(Vertex), offsetof(Vertex, normal)},
            {3, 3 , GL_FLOAT, 0, sizeof(Vertex), offsetof(Vertex, tangent)},
            {4, 3 , GL_FLOAT, 0, sizeof(Vertex), offsetof(Vertex, bitangent)}
            }), 
        
        VBO(GL_ARRAY_BUFFER), EBO(GL_ELEMENT_ARRAY_BUFFER)
        {}
    } m;

    void apply_transform();
    void join_identical_vertices();

    void compute_AABB();

    void onFrustum(const frustum& F);
    bool isInstance(const Mesh& M) const;

    bool operator==(const Mesh& M) const 
    {
        return isInstance(M); 
    }

    void init();
    void render();

};
