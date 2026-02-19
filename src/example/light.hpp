#pragma once

#include <cstdint>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 

struct Light
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 attenuation;
    glm::vec3 color;
    float inner_cone;
    float outer_cone;
    uint32_t type;
    int shadow;
};