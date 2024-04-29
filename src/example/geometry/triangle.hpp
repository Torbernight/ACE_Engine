#pragma once

#include "AABB.hpp"
#include "plane.hpp"

struct triangle
{
    glm::vec3 A;
    glm::vec3 B;
    glm::vec3 C;

    glm::vec3 get_normal() const 
    {
        glm::vec3 AB = B - A;
        glm::vec3 CB = C - B;
        return glm::normalize(glm::cross(AB,CB));
    }

    plane get_plane() const 
    {
        return plane(get_normal(), A);
    }

    bool test_point(const glm::vec3& X, float epsilon = 0.01f) const 
    {
        using namespace glm;
        vec3 AB = B - A;
        vec3 BC = C - B;
        vec3 CA = A - C;
        plane P = plane(normalize(cross(AB,BC)), A);

        if (std::abs(P.sdist(X)) > epsilon)
            return false;

        plane PCA = plane(normalize(AB), A);
        plane PAB = plane(normalize(BC), B);
        plane PBC = plane(normalize(CA), C);

        return PAB.testPoint(X) && PBC.testPoint(X) && PCA.testPoint(X);
    }

    bool test_point(const glm::vec3& X, glm::vec3& normal, float epsilon = 0.1f) const 
    {
        using namespace glm;
        vec3 AB = B - A;
        vec3 BC = C - B;
        vec3 CA = A - C;
        plane P = plane(normalize(cross(AB,BC)), A);
        normal = P.normal;

        if (std::abs(P.sdist(X)) > epsilon)
            return false;

        plane PCA = plane(normalize(AB), A);
        plane PAB = plane(normalize(BC), B);
        plane PBC = plane(normalize(CA), C);

        return PAB.testPoint(X) && PBC.testPoint(X) && PCA.testPoint(X);
    }

    bool test_AABB(const AABB& box, glm::vec3& normal) const
    {
        using namespace glm;

        vec3 AB = normalize(B - A);
        vec3 BC = normalize(C - B);
        vec3 CA = normalize(A - C);

        plane P = plane(normalize(cross(AB,BC)), A);
        normal = P.normal;

        vec3 c = box.get_center();
        vec3 e = box.get_extent();

        if (!P.AABB_intersection(e, c))
            return false;

        plane PCA = plane(AB, A);
        plane PAB = plane(BC, B);
        plane PBC = plane(CA, C);

        return 
        PAB.isOnOrForwardPlane(e, c) && 
        PBC.isOnOrForwardPlane(e, c) && 
        PCA.isOnOrForwardPlane(e, c);
    }

    glm::vec3 get_Baricenter() const
    {
        return (A+B+C)/3.0f;
    }

};