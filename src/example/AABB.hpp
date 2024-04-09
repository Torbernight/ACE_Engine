#pragma once

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/vector_relational.hpp>

struct AABB
{
        glm::vec3 min, max;

        AABB() = default;

        AABB(const glm::vec3& m, const glm::vec3& M) :
        min(m), max(M) {}

        AABB(const AABB& A, const AABB& B)
        {
                this->min.x = glm::min(A.min.x, B.min.x);
                this->min.y = glm::min(A.min.y, B.min.y);
                this->min.z = glm::min(A.min.z, B.min.z);
                this->max.x = glm::max(A.max.x, B.max.x);
                this->max.y = glm::max(A.max.y, B.max.y);
                this->max.z = glm::max(A.max.z, B.max.z);
        }

        glm::vec3 get_center() const
        {
                return (min+max)*0.5f;
        }

        glm::vec3 get_extent() const
        {
                return max-min;
        }

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

        AABB& operator*(const glm::mat4& M) 
        {
                min = glm::vec3( M * glm::vec4(min, 1.0f) );
                max = glm::vec3( M * glm::vec4(max, 1.0f) );
                return *this;
        }

        AABB& operator*(const glm::mat3& M) 
        {
                min = M * min;
                max = M * max;
                return *this;
        }
};
