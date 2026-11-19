#include "object.hpp"
#include "camera.hpp"
#include "scene/scene.hpp"

void DynamicObject::grab(void* player)
{
    Camera& P = *static_cast<Camera*>(player);
    if (P.m.grabbed_object)
        return;
    P.m.grabbed_object = this;
    this->m_grabbed = true;
    this->m_selected = false;

}
void DynamicObject::use(void* player)
{
    Camera& P = *static_cast<Camera*>(player);
    this->m_grabbed = false;
    P.m.grabbed_object = nullptr;
}

void DynamicObject::select(void* player)
{
    Camera& P = *static_cast<Camera*>(player);

    if (((void*)P.m.pointed_object) != ((void*)this) && P.m.pointed_object){
        P.m.pointed_object->m_selected = false;
    }
    this->m_selected = true;
    P.m.pointed_object = this;
}

void DynamicObject::update(void* player, void* scene, float delta)
{
    if (m_grabbed)
        return;
    vel.y -= 9.8f*delta;
    Scene& S = *static_cast<Scene*>(scene);
    Renderer::Mesh &M = S.Meshes[m.mesh];
    M.m.transform[3].y += vel.y*delta;
}

void DynamicObject::update_grabbed(void* player, void* scene)
{
    if (!m_grabbed)
        return;

    Camera& P = *static_cast<Camera*>(player);
    Scene& S = *static_cast<Scene*>(scene);
    Renderer::Mesh &M = S.Meshes[m.mesh];
    glm::vec3 pos = P.m.pos - P.m.up*0.125f + P.m.front*0.6f + P.m.right*0.1f;
    glm::mat4 trans = P.get_transform();
    trans[3] = glm::vec4(pos, 1.0f);
    M.m.transform = trans;
    return;
}
