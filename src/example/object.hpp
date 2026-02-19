#pragma once
#include <vector>
#include "mesh.hpp"
#include "geometry/triangle.hpp"

class Object
{
    public:

    virtual void grab(void*)
    {
        return;
    }
    virtual void use(void*)
    {
        return;
    }
    virtual void select(void*)
    {
        return;
    }
    virtual void update(void*, float)
    {
        return;
    }

    struct members
    {
        unsigned int Mesh;
        glm::mat4 transform;

        members() = default;
        members(unsigned int M) : Mesh(M), transform(1.0f){}
        members(unsigned int M, const glm::mat4& T) : Mesh(M), transform(T){}
    } m;

    bool m_selected = false;
    bool m_grabbed = false;

    Object() = default;

    Object(bool collision, bool static_mesh, bool grabbable, bool selectable, bool selected, bool grabbed) : 
    m_selected(selected), m_grabbed(grabbed), m_collision(collision), m_static_mesh(static_mesh), m_grabbable(grabbable), m_selectable(selectable)
    {}
    Object(unsigned int M, bool collision, bool static_mesh, bool grabbable, bool selectable, bool selected, bool grabbed) : 
    m(M), m_selected(selected), m_grabbed(grabbed), m_collision(collision), m_static_mesh(static_mesh), m_grabbable(grabbable), m_selectable(selectable)
    {}
    protected:
    const bool m_collision = true;
    const bool m_static_mesh = true;
    const bool m_grabbable = false;
    const bool m_selectable = false;

};

class StaticObject : public Object
{
    public:
    StaticObject() : 
    Object(true, true, false, false, false, false)
    {}

    StaticObject(unsigned int M) : 
    Object(M, true, true, false, false, false, false)
    {}

    void grab(void*)
    {
        return;
    }
    void use(void*)
    {
        return;
    }
    void select(void*)
    {
        return;
    }
    void update(void*, float)
    {
        return;
    }
    std::vector<triangle> collision_mesh;
    
};

class DynamicObject : public Object
{
    public:

    DynamicObject() :
    Object(true, false, true, true, false, false)
    {}

    DynamicObject(unsigned int M) : 
    Object(M, true, true, false, false, false, false)
    {}

    glm::vec3 vel = glm::vec3(0.0f);

    void grab(void* player);
    void use(void* player);
    void select(void* player);
    void update(void* player, void* Scene, float delta);
    void update_grabbed(void* player, void* Scene);
};