#include "camera.hpp"

static constexpr glm::vec3 world_up = glm::vec3(0.0,1.0,0.0);

inline static
glm::vec3 move2D(const glm::vec3& v)
{
    return (glm::vec3(v.x, 0.0f, v.z)/hypotf(v.x,v.z));
}

void camera::move(float dt)
{
    constexpr glm::vec3 world_up = glm::vec3(0,1,0);
    glm::vec3 speed = glm::vec3(0.0);

    if(movement.forward)
        speed += move2D(front);
    if(movement.backward)
        speed -= move2D(front);
    if(movement.left)
        speed -= move2D(right);
    if(movement.right)
        speed += move2D(right);
    if(movement.up)
        speed += world_up;
    if(movement.down)
        speed -= world_up;
    
    if (glm::length(speed) == 0)
        return;

    speed = glm::normalize(speed);
    pos += speed*dt;
}

void camera::makeView()
{
    view = glm::lookAt(pos, pos+front, glm::vec3(0.0,1.0,0.0));
}

void camera::rotate()
{

    rotation.x = fmodf(rotation.x, 2*M_PIf);
    //if (rotation.x < 0) rotation.x += 2*M_PIf;
    rotation.y = glm::clamp(rotation.y, -M_PI_2f*0.98f, M_PI_2f*0.98f);

    glm::vec2 c(glm::cos(rotation)), s(glm::sin(rotation));

    front.x = s.x*c.y;
    front.y = s.y;
    front.z = -c.x*c.y;

    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, world_up));
    up    = glm::normalize(glm::cross(right, front));
}