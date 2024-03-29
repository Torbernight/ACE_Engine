#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

class camera
{
    public:
  
    struct movement
    {
        uint8_t forward  = 0;
        uint8_t backward = 0;
        uint8_t right    = 0;
        uint8_t left     = 0;
        uint8_t up       = 0;
        uint8_t down     = 0;
        uint8_t sprint   = 0;
    } movement;


    void move(float dt);

    void rotate(const glm::vec2& r, float dt);

    void set_update(bool update)
    {
        m.update = update;
    }

    glm::vec3& getPos(){
        return m.pos;
    }

    glm::vec3& getFront(){
        return m.front;
    }

    void makeView();
    glm::mat4& getView()
    {
        return m.view;
    }
    void makeProj(float aspect);
    glm::mat4 getProj()
    {
        return m.proj;
    }

    glm::vec3& getVel()
    {
        return m.vel;
    }

    void setPos(const glm::vec3& pos)
    {
        m.pos = pos;
    }

    private:

    struct members{
        glm::vec3 front = glm::vec3(0.0,0.0,-1.0);
        glm::vec3 right = glm::vec3(1.0,0.0,0.0);
        glm::vec3 up = glm::vec3(0.0,1.0,0.0);
        glm::vec2 rotation = glm::vec2(0.0);
        glm::vec3 pos = glm::vec3(0.0, 0.0, -4.0);
        glm::vec3 vel = glm::vec3(0.0);
        
        float FOVy = glm::radians(70.0f);
        float zNear = 0.1f;
        float zFar = 1000.0f;

        glm::mat4 view = glm::mat4(1);
        glm::mat4 proj = glm::mat4(1);
        
        bool update = false;

    } m;

    inline static float s_walk_speed = 1.5f;
    inline static float s_sprint_speed = 3.0f;

};