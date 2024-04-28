#pragma once

#include "scene.hpp"
#include "camera.hpp"
#include "object.hpp"

void test_player_collision(camera &player, scene* gameScene, float tick_rate);
void test_narrow_collision(const static_object& obj, camera& player, float tick_rate);
void resolve_collision(const triangle &tri, const vec3& normal, camera& player, float tick_rate);

void test_object_collision(scene* gameScene, float tick_rate);
void resolve_collision(const AABB& box, dynamic_object& Obj, const AABB& hitbox, scene* gameScene, float tick_rate);