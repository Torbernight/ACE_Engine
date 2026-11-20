#include <execution>
#include <algorithm>

#include "physics.hpp"

inline static 
float maxcomp(const glm::vec3& V){
    return glm::max(abs(V.x), glm::max(abs(V.y),abs(V.z)));
}
inline static
void clampnan(glm::vec3 &V){
    V.x = glm::isnan(V.x) ? 0.0 : glm::isinf(V.x) ? glm::sign(V.x) : V.x;
    V.y = glm::isnan(V.y) ? 0.0 : glm::isinf(V.y) ? glm::sign(V.y) : V.y;
    V.z = glm::isnan(V.z) ? 0.0 : glm::isinf(V.z) ? glm::sign(V.z) : V.z;
}


void test_player_collision(Player &player, Scene* gameScene, float tick_rate)
{
    using namespace glm;

    if (length(player.m.realVel) == 0)
        return;
    
    std::mutex mutex;
    player.m.collision = false;
    player.m.on_floor_last_frame = player.m.floor_collision;
    player.m.floor_collision = false;

    //TODO some kind of space partitioning
    std::for_each(std::execution::par, gameScene->static_objects.begin(), gameScene->static_objects.end(),
    [&player, gameScene, tick_rate, &mutex](const StaticObject& Obj)
    {
        auto &M = gameScene->Meshes[Obj.m.mesh];
        const mat4& transform = M.m.transform;
        AABB hitbox = M.m.bounding_box * transform;
        if (player.m.bounding_box.test_AABB(hitbox)){
            mutex.lock();
            test_narrow_collision(Obj, player, tick_rate);
            mutex.unlock();
        }
    }
    );
}


void test_narrow_collision(const StaticObject& obj, Player& player, float tick_rate)
{
    using namespace glm;
    for (const auto &tri : obj.collision_mesh)
    {
            vec3 normal;
            if (tri.test_AABB(player.m.bounding_box, normal))
            resolve_collision(tri, normal, player, tick_rate);
    }
}

void resolve_collision(const triangle &tri, const glm::vec3& normal, Player& player, float tick_rate)
{
    using namespace glm;

    player.m.collision = true;
    player.m.floor_collision = normal.y > 0.1;

    plane P = plane(normal, tri.A);
    vec3 c = player.m.bounding_box.get_center();
    vec3 e = player.m.bounding_box.get_extent();
    float t = std::abs(dot(abs(normal), e) - P.sdist(c));

    player.m.vel += normal*dot(player.m.vel,-normal);
    //player.m.pos = player.m.previousPos + player.m.vel*tick_rate;
    player.m.pos += normal*t;

    player.m.realVel = (player.m.pos - player.m.previousPos) / tick_rate;
    player.make_AABB();
}

void test_object_collision(Scene* gameScene, float tick_rate)
{
    std::mutex mutex;
    for (auto &Obj : gameScene->dynamic_objects)
    {
        if (Obj.m_grabbed)
            continue;
        auto &M = gameScene->Meshes[Obj.m.mesh];
        AABB box = M.m.bounding_box * M.m.transform;
        auto begin = gameScene->static_objects.begin();
        auto end = gameScene->static_objects.end();
        std::for_each(std::execution::par,begin,end,
        [gameScene, tick_rate, &mutex, &box, &M, &Obj](const StaticObject& static_obj)
        {
            auto &Mesh = gameScene->Meshes[static_obj.m.mesh];
            const glm::mat4& transform = Mesh.m.transform;
            AABB hitbox = Mesh.m.bounding_box * transform;
            if (box.test_AABB(hitbox))
            {
                mutex.lock();
                resolve_collision(box, Obj, hitbox, gameScene, tick_rate);
                box = M.m.bounding_box * M.m.transform;
                mutex.unlock();
            }
        }
    );
    }
}

void resolve_collision(const AABB& box, DynamicObject& Obj, const AABB& hitbox, Scene* gameScene, float tick_rate)
{
    auto &M = gameScene->Meshes[Obj.m.mesh];
    glm::vec3 dir = (box.get_center() - hitbox.get_center()) / hitbox.get_extent();
    clampnan(dir);
    dir = glm::trunc(dir/maxcomp(dir));
    glm::vec3 Pos = glm::vec3(M.m.transform[3]);
    glm::vec3 previousPos = Pos - Obj.vel*tick_rate;
    Obj.vel *= (1.0f-glm::abs(dir));
    Pos = previousPos + Obj.vel * tick_rate;
    //SDL_assert(!glm::any(glm::isnan(Pos)) && !glm::any(glm::isinf(Pos)));
    M.m.transform[3] = glm::vec4(Pos, 1.0f);
}