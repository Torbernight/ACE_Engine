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
    std::vector<glm::mat4> transforms;
    unsigned int nInstances = 1;
    unsigned int nVisibleInstances = 1;
    unsigned int materialIndex = 0;
    bool discard = false;

    void apply_transform();
    void join_identical_vertices();

    void onFrustum(const frustum& F)
    {
        if (nInstances == 1) 
        {
            AABB box = bounding_box * transforms[0];
            bool onView = F.testAABB(box);
            nVisibleInstances = onView ? 1 : 0;
            return;
        }

        std::vector<unsigned int> indices;
        indices.reserve(nInstances);
        std::vector<glm::mat4> tCopy(transforms);

        for(unsigned int n = 0; n < nInstances; n++)
        {
            AABB box = bounding_box * transforms[n];
            bool onView = F.testAABB(box);
            if (onView) {
                indices.push_back(n);
            }
        }
        nVisibleInstances = indices.size();
        auto start = indices.begin();
        auto end = indices.end();
        for (unsigned int n = 0; n < nInstances; n++)
        {
            if (!std::binary_search(start, end, n)){
                indices.push_back(n);
            }
        }
        for(unsigned int n = 0; n < nInstances; n++)
        {
            tCopy[n] = transforms[indices[n]];
        }
        transforms = tCopy;
    }

    //check if instance
    bool isInstance(const mesh& M) const
    {
        static const std::regex pattern("\\.[0-9][0-9][0-9]");

        std::string subName0 = std::regex_replace(Name,pattern,"");
        std::string subName1 = std::regex_replace(M.Name,pattern,"");
        
        return subName0 == subName1 &&
            vertices.size() == M.vertices.size() &&
            indices.size() == M.indices.size() &&
            materialIndex == M.materialIndex;
    }

    //instance check operator
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
