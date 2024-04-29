#pragma once

#include "AABB.hpp"

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

    float ray_intersection(const glm::vec3& origin, const glm::vec3& dir) const
    {
        using namespace glm;
        float denominator = dot(dir, normal);
        if (denominator == 0.0f)
            return (1.0/0.0);
        
        return (d - dot(origin, normal)) / denominator;
    }    

    glm::vec3 nearest_point(const glm::vec3 &P) const
    {
        float d = (sdist(P));

        if (d == 0){
            return P;
        }
        return P+normal*glm::sign(d)*ray_intersection(P,normal);

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

    bool isOnOrForwardPlane(const glm::vec3& extent, const glm::vec3& center) const
    {
        return sdist(center)+glm::dot(glm::abs(normal), extent) >= 0;
    }


    bool isOnOrForwardPlane(const AABB& box) const
    {
        glm::vec3 e = box.get_extent();
        glm::vec3 c = box.get_center();
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        return sdist(c)+glm::dot(glm::abs(normal), e) >= 0;
    }

    bool AABB_intersection(const AABB& box) const 
    {
        glm::vec3 e = box.get_extent();
        glm::vec3 c = box.get_center();
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        return std::abs(sdist(c)) <= glm::dot(glm::abs(normal), e);
    }

    bool AABB_intersection(const glm::vec3& extent, const glm::vec3& center) const 
    {
        return std::abs(sdist(center)) <= glm::dot(glm::abs(normal), extent);
    }


};