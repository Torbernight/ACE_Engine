#pragma once

#include <vector>
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class obj
{
    public:

    void import(const std::filesystem::path&);
    
    void smooth_normals();

    void gen_index();

    struct vertex
    {
        glm::vec3 pos;
        glm::vec2 texcoord;
        glm::vec3 normal;
    };

    void addVertex(const glm::vec3& pos, const glm::vec2& texcoord, const glm::vec3& normal)
    {
        m.vertices.push_back({pos,texcoord,normal});
    }

    std::vector<vertex>& getVertices()
    {
        return m.vertices;
    }

    std::vector<unsigned int>& getIndices()
    {
        return m.indices;
    }


    private:
        struct members
        {
            std::vector<vertex> vertices;
            std::vector<unsigned int> indices;
        } m;
};