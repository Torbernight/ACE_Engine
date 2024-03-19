#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct camera
{
    struct movement
    {
        bool forward  = false;
        bool backward = false;
        bool right    = false;
        bool left     = false;
        bool up       = false;
        bool down     = false;
    } movement;

    glm::vec3 front = glm::vec3(0.0,0.0,-1.0);
    glm::vec3 right = glm::vec3(1.0,0.0,0.0);
    glm::vec3 up = glm::vec3(0.0,1.0,0.0);
    glm::vec2 rotation = glm::vec2(0.0);
    glm::vec3 pos = glm::vec3(0.0, 0.0, -4.0);
    glm::mat4 view = glm::mat4(1);
    bool update = false;

    void move(float dt);

    void rotate();

    void makeView();

};