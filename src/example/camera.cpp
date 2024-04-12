#include "camera.hpp"

static constexpr glm::vec3 world_up = glm::vec3(0.0,1.0,0.0);

inline static
glm::vec3 move2D(const glm::vec3& v)
{
    return glm::normalize(glm::vec3(v.x, 0.0f, v.z));
}

void camera::makeProj()
{
    m.proj = glm::perspective(m.FOVy, m.aspect, m.zNear, m.zFar);
    m.view_frustum.create(m.pos, m.front, m.up, m.right, m.zNear, m.zFar, m.FOVy, m.aspect);
}

void camera::move(float dt)
{
    m.vel = 
    (float)(movement.forward-movement.backward)*move2D(m.front)
    + 
    (float)(movement.right-movement.left)*move2D(m.right)
    +
    (float)(movement.up-movement.down)*world_up;

    if (glm::dot(m.vel,m.vel) == 0)
        return;

    m.vel = glm::normalize(m.vel) * glm::mix(s_walk_speed, s_sprint_speed, (float)movement.sprint);

    m.pos += m.vel*dt;

    make_AABB();
}

void camera::make_AABB()
{
    glm::vec3 bcenter = glm::vec3(m.pos.x, m.pos.y-0.845f, m.pos.z);
    glm::vec3 bbounds = glm::vec3(0.25f, 0.845f, 0.25f);
    this->m.bounding_box = AABB(bcenter-bbounds, bcenter+bbounds);
}

void camera::makeView()
{
    m.view = glm::lookAt(m.pos, m.pos+m.front, world_up);
    m.view_frustum.create(m.pos, m.front, m.up, m.right, m.zNear, m.zFar, m.FOVy, m.aspect);
}

void camera::rotate(const glm::vec2& r, float dt)
{
    m.rotation += r*dt;

    m.rotation.x = fmodf(m.rotation.x, 2*M_PIf);
    m.rotation.y = glm::clamp(m.rotation.y, -M_PI_2f*0.98f, M_PI_2f*0.98f);

    glm::vec2 c(glm::cos(m.rotation)), s(glm::sin(m.rotation));

    m.front.x = s.x*c.y;
    m.front.y = s.y;
    m.front.z = -c.x*c.y;

    m.front = glm::normalize(m.front);
    m.right = glm::normalize(glm::cross(m.front, world_up));
    m.up    = glm::normalize(glm::cross(m.right, m.front));
}