#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "frustum.hpp"
#include "object.hpp"

class camera
{
    public:


    struct movement
    {
        uint8_t forward    = 0;
        uint8_t backward   = 0;
        uint8_t right      = 0;
        uint8_t left       = 0;
        uint8_t up         = 0;
        uint8_t down       = 0;
        uint8_t sprint     = 0;
        uint8_t lclick     = 0;
        uint8_t hasClicked = 0;
        uint8_t use_key    = 0;
        uint8_t use        = 0;
    } movement;


    void move(float dt);

    void rotate(float dt);

    void reset_rotation() {
        m.delta_rotation = glm::vec2(0);
    }

    void set_update(bool update)
    {
        m.update = update;
    }

    glm::vec3& getPos(){
        return m.pos;
    }

    const glm::vec3& getPos() const
    {
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
    void makeProj();

    glm::mat4 getProj()
    {
        return m.proj;
    }

    glm::vec3& getVel()
    {
        return m.vel;
    }

    float getNear() const
    {
        return m.zNear;
    }

    float getFar() const
    {
        return m.zFar;
    }

    float getFOV() const 
    {
        return m.FOVy;
    }

    void setFOV(float fov)
    {
        m.FOVy = fov;
        makeProj();
    }

    void setPos(const glm::vec3& pos)
    {
        m.pos = pos;
        make_AABB();
    }

    void setAspect(float aspect)
    {
        m.aspect = aspect;
    }

    void make_AABB();

    float footstep_offset() const;

    glm::mat4 get_transform() const;

    bool should_sound_footstep() const
    {
        return m.jump || (m.moving && std::abs(footstep_offset()+m.footstep_amp) <= 0.001);
    }

    bool has_grabbed_object() const
    {
        return m.grabbed_object != nullptr;
    }

    struct members{
        glm::vec3 front = glm::vec3(0.0,0.0,-1.0);
        glm::vec3 right = glm::vec3(1.0,0.0,0.0);
        glm::vec3 up = glm::vec3(0.0,1.0,0.0);
        glm::vec2 rotation = glm::vec2(0.0);
        glm::vec2 delta_rotation = glm::vec2(0.0);
        glm::vec3 pos = glm::vec3(0.0, 0.0, -4.0);
        glm::vec3 vel = glm::vec3(0.0);
        glm::vec3 acc = glm::vec3(0.0);
        
        float aspect = 16.0/9.0;
        float FOVy = glm::radians(50.0f);
        float zNear = 0.1f;
        float zFar = 1000.0f;

        glm::mat4 view = glm::mat4(1);
        glm::mat4 proj = glm::mat4(1);
        
        bool update = false;
        frustum view_frustum;
        AABB bounding_box;
        bool collision = false;
        bool floor_collision = false;
        bool on_floor_last_frame = false;
        float footstep_timer = 0.0f;
        float footstep_amp = 0.0f;
        float footstep_freq = 3.0f;
        bool moving = false;
        bool jump = false;

        dynamic_object* grabbed_object = nullptr;
        dynamic_object* pointed_object = nullptr;

        glm::vec3 previousPos = glm::vec3(0.0);
        glm::vec3 realVel = glm::vec3(0.0);

    } m;

    private:

    inline static float s_walk_speed = 3.0f;
    inline static float s_sprint_speed = 6.0f;

};