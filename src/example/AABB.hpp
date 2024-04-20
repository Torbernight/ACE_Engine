#pragma once

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

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
                return (max-min)*0.5f;
        }

        bool test_point(const glm::vec3& v)
        {
                return 
                v.x >= min.x &&
                v.y >= min.y &&
                v.z >= min.z &&
                v.x <= max.x &&
                v.y <= max.y &&
                v.z <= max.z;
                //glm::all(glm::greaterThanEqual(v, min)) 
                //&& 
                //glm::all(glm::lessThanEqual(v, max));
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

        AABB operator*(const glm::mat4& M) const
        {
                using namespace glm;
                vec3 c = get_center() + vec3(M[3]);
                vec3 e = get_extent();

                vec3 right   = abs(vec3(M[0]) * e.x);
                vec3 up      = abs(vec3(M[1]) * e.y);
                vec3 forward = abs(vec3(M[2]) * e.z);

                e = vec3(
                right.x + up.x + forward.x,
                right.y + up.y + forward.y,
                right.z + up.z + forward.z
                );

                return AABB(c-e, c+e);
        }
};