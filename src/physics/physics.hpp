#pragma once

#include "../scene/scene.hpp"
#include "../player.hpp"
#include "../scene/object.hpp"

void test_player_collision(Player &player, Scene* gameScene, float tick_rate);
void test_narrow_collision(const StaticObject& obj, Player& player, float tick_rate);
void resolve_collision(const triangle &tri, const glm::vec3& normal, Player& player, float tick_rate);

void test_object_collision(Scene* gameScene, float tick_rate);
void resolve_collision(const AABB& box, DynamicObject& Obj, const AABB& hitbox, Scene* gameScene, float tick_rate);