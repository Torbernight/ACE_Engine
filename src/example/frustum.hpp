#pragma once

#include "AABB.hpp"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

struct plane
{
    glm::vec3 normal;
    float d;

    plane() = default;

    plane(const glm::vec3& n, float dist): normal(glm::normalize(n)), d(dist) {}

    plane(const glm::vec3& n, const glm::vec3& p): normal(glm::normalize(n))
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
};

struct frustum
{
    plane left, right, top, bottom, near, far;

    bool testPoint(const glm::vec3& P) const
    {
        return
        left.testPoint(P)  &&
        right.testPoint(P) &&
        top.testPoint(P)    &&
        bottom.testPoint(P)  &&
        near.testPoint(P)  &&
        far.testPoint(P);

    }

    bool testAABB(const AABB& B) const
    {
        using namespace glm;
        vec3 c = B.get_center();
        vec3 e = B.get_extent();
        vec3 Points[8] = 
        {
            c+e,
            c-e,
            c+vec3(-e.x, e.y, e.z),
            c+vec3( e.x,-e.y, e.z),
            c+vec3( e.x, e.y,-e.z),
            c+vec3(-e.x,-e.y, e.z),
            c+vec3(-e.x, e.y,-e.z),
            c+vec3( e.x,-e.y,-e.z)
        };
        bool test =
        testPoint(Points[0]) || 
        testPoint(Points[1]) || 
        testPoint(Points[2]) || 
        testPoint(Points[3]) || 
        testPoint(Points[4]) || 
        testPoint(Points[5]) || 
        testPoint(Points[6]) || 
        testPoint(Points[7]);
        return test;
    }

    void create(const glm::vec3& camPos, const glm::vec3& camForward, const glm::vec3& camUp, const glm::vec3& camRight, float nearDistance, float farDistance, float fov, float aspect)
    {
        using namespace glm;

        vec3 nearCenter = camPos + camForward * nearDistance;
        vec3 farCenter = camPos + camForward * farDistance;

        float tan_fov2 = 2 * tanf(fov/ 2);

        float nearHeight = tan_fov2  * nearDistance;
        float farHeight = tan_fov2 * farDistance;
        float nearWidth = nearHeight * aspect;
        float farWidth = farHeight * aspect;

        vec3 farTopLeft = farCenter + camUp * (farHeight*0.5f) - camRight * (farWidth*0.5f);
        vec3 farTopRight = farCenter + camUp * (farHeight*0.5f) + camRight * (farWidth*0.5f);
        vec3 farBottomLeft = farCenter - camUp * (farHeight*0.5f) - camRight * (farWidth*0.5f);
        vec3 farBottomRight = farCenter - camUp * (farHeight*0.5f) + camRight * (farWidth*0.5f);

        vec3 nearTopLeft = nearCenter + camUp * (nearHeight*0.5f) - camRight * (nearWidth*0.5f);
        vec3 nearTopRight = nearCenter + camUp * (nearHeight*0.5f) + camRight * (nearWidth*0.5f);
        vec3 nearBottomLeft = nearCenter - camUp * (nearHeight*0.5f) - camRight * (nearWidth*0.5f);
        vec3 nearBottomRight = nearCenter - camUp * (nearHeight*0.5f) + camRight * (nearWidth*0.5f);

        vec3 p0, p1, p2;
        p0 = nearBottomLeft; 
        p1 = farBottomLeft; 
        p2 = farTopLeft;
        left.normal = normalize(cross(p1-p0, p2-p1));
        left.d = dot(left.normal, p0);

        p0 = nearTopLeft; 
        p1 = farTopLeft; 
        p2 = farTopRight;
        top.normal = normalize(cross(p1-p0, p2-p1));
        top.d = dot(top.normal , p0);

        p0 = nearTopRight; 
        p1 = farTopRight; 
        p2 = farBottomRight;
        right.normal = normalize(cross(p1-p0, p2-p1));
        right.d = dot(right.normal , p0);

        p0 = nearBottomRight; 
        p1 = farBottomRight; 
        p2 = farBottomLeft;
        bottom.normal = normalize(cross(p1-p0, p2-p1));
        bottom.d = dot(bottom.normal , p0);

        near.normal = camForward;
        near.d = dot(near.normal, nearCenter);
        far.normal = -camForward;
        far.d = dot(far.normal, farCenter);
    }


};