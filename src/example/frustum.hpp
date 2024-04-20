#pragma once

#include "AABB.hpp"
#include "plane.hpp"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>

struct frustum
{
    plane left, right, top, bottom, near, far;

    bool testPoint(const glm::vec3& P) const
    {
        return
        left.testPoint(P)  &&
        right.testPoint(P) &&
        top.testPoint(P)   &&
        bottom.testPoint(P)&&
        near.testPoint(P)  &&
        far.testPoint(P);
    }

    bool testAABB(const AABB& B) const
    {
        return 
            left.isOnOrForwardPlane(B) &&
            right.isOnOrForwardPlane(B) &&
            top.isOnOrForwardPlane(B) &&
            bottom.isOnOrForwardPlane(B) &&
            near.isOnOrForwardPlane(B) &&
            far.isOnOrForwardPlane(B);
    }

    static glm::vec3 ncross(const glm::vec3& a, const glm::vec3& b)
    {
        return glm::normalize(glm::cross(a,b));
    }

    void create(const glm::vec3& camPos, const glm::vec3& camForward, const glm::vec3& camUp, const glm::vec3& camRight, float nearDistance, float farDistance, float fov, float aspect)
    {
        using namespace glm;

        float halfVSide = farDistance * tanf(fov * 0.5f);
        float halfHSide = halfVSide * aspect;
        vec3 frontMultFar = farDistance * camForward;

        near.normal = camForward;
        near.d = dot(camPos + nearDistance * camForward, near.normal);
        far.normal = -camForward;
        far.d = dot(camPos + frontMultFar, far.normal);
        right.normal = ncross(frontMultFar - camRight * halfHSide, camUp);
        right.d = dot(camPos, right.normal);
        left.normal =  ncross(camUp,frontMultFar + camRight * halfHSide);
        left.d = dot(camPos, left.normal);
        top.normal = ncross(camRight, frontMultFar - camUp * halfVSide);
        top.d = dot(camPos, top.normal);
        bottom.normal = ncross(frontMultFar + camUp * halfVSide, camRight);
        bottom.d = dot(camPos, bottom.normal);
    }

};