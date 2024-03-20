#pragma once

#include <glm/vec3.hpp>

struct light
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 attenuation;
    glm::vec3 color_diffuse;
    glm::vec3 color_specular;
    glm::vec3 color_ambient;
    float inner_cone;
    float outer_cone;
};