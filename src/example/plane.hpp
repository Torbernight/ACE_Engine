#pragma once

#include "AABB.hpp"
#include <glm/vec3.hpp>

struct plane
{
    glm::vec3 normal;
    float d;

    plane() = default;

    plane(const glm::vec3& n, float dist): 
    normal(glm::normalize(n)), d(dist) 
    {}

    plane(const glm::vec3& n, const glm::vec3& p): 
    normal(glm::normalize(n))
    {
        d = glm::dot(n,p);
    }

    float sdist(const glm::vec3 &p) const
    {
        return glm::dot(normal,p) - d;
    }

    bool testPoint(const glm::vec3 &p) const
    {
        return sdist(p) >= 0;
    }

    bool intersects_triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
    {
        float sa = glm::sign(sdist(A));
        float sb = glm::sign(sdist(B));
        float sc = glm::sign(sdist(C));
        return sa == sb && sb == sc && sc != 0.0f;
    }

    bool isOnOrForwardPlane(const AABB& box) const
    {
        glm::vec3 e = box.get_extent();
        glm::vec3 c = box.get_center();
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        return sdist(c)+glm::dot(glm::abs(normal), e) >= 0;
    }

};