#pragma once

#include <glm/vec3.hpp>
#include <glm/ext/vector_relational.hpp>

struct AABB
{
        glm::vec3 min, max;

        AABB() = default;

        AABB(const glm::vec3& m, const glm::vec3& M) :
        min(m), max(M) {}

        bool test_point(const glm::vec3& v)
        {
                return 
                glm::all(glm::greaterThanEqual(v, min)) 
                && 
                glm::all(glm::lessThanEqual(v, max));
        }

        bool test_AABB(const AABB& B)
        {
                return
                min.x <= B.max.x &&
                max.x >= B.min.x &&
                min.y <= B.max.y &&
                max.y >= B.min.y &&
                min.z <= B.max.z &&
                max.z >= B.min.z;
        }
};