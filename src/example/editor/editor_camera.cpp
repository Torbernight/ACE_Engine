#include "editor_camera.hpp"

static constexpr glm::vec3 world_up = glm::vec3(0.0,1.0,0.0);

inline static
glm::vec3 move2D(const glm::vec3& v)
{
    return glm::normalize(glm::vec3(v.x, 0.0f, v.z));
}

void editor_camera::move(float dt)
{
    using namespace glm;

    m.vel = 
    (float)(movement.forward-movement.backward)*move2D(m.front)
    + 
    (float)(movement.right-movement.left)*move2D(m.right)
    + 
    (float)(movement.up-movement.down)*world_up;

    m.moving = m.vel != vec3(0.0);

    float speed = length(m.vel);
    float speedcap = mix(s_walk_speed, s_sprint_speed, (float)movement.sprint);

    if (speed == 0)
        return;

    m.vel = normalize(m.vel)*speedcap;

    m.previousPos = m.pos;
    m.pos += m.vel*dt;
}