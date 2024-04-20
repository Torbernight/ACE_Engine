#include "object.hpp"
#include "camera.hpp"
#include "scene.hpp"

void dynamic_object::grab(void* player)
{
    camera& P = *static_cast<camera*>(player);
    if (P.m.grabbed_object)
        return;
    P.m.grabbed_object = this;
    this->m_grabbed = true;
    this->m_selected = false;

}
void dynamic_object::use(void* player)
{
    camera& P = *static_cast<camera*>(player);
    this->m_grabbed = false;
    P.m.grabbed_object = nullptr;
}

void dynamic_object::select(void* player)
{
    camera& P = *static_cast<camera*>(player);

    if (((void*)P.m.pointed_object) != ((void*)this) && P.m.pointed_object){
        P.m.pointed_object->m_selected = false;
    }
    this->m_selected = true;
    P.m.pointed_object = this;
}

void dynamic_object::update(void* player, void* Scene, float delta)
{
    if (m_grabbed)
        return;
    vel.y -= 9.8f*delta;
    scene& S = *static_cast<scene*>(Scene);
    mesh &M = S.Meshes[m.Mesh];
    M.m.transform[3].y += vel.y*delta;
}

void dynamic_object::update_grabbed(void* player, void* Scene)
{
    if (!m_grabbed)
        return;

    camera& P = *static_cast<camera*>(player);
    scene& S = *static_cast<scene*>(Scene);
    mesh &M = S.Meshes[m.Mesh];
    glm::vec3 pos = P.m.pos - P.m.up*0.125f + P.m.front*0.6f + P.m.right*0.1f;
    glm::mat4 trans = P.get_transform();
    trans[3] = glm::vec4(pos, 1.0f);
    M.m.transform = trans;
    return;
}
