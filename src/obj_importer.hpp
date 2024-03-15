#pragma once

#include <vector>
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class obj
{
    public:

    void import(const std::filesystem::path&);
    
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

    private:
        struct members
        {
            std::vector<vertex> vertices;
        } m;
};