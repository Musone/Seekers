#pragma once

#include "../ecs/Entity.hpp"
#include "../app/Components.hpp"
#include "../ecs/Registry.hpp"

class CollisionSystem
{
public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    // Check for collisions between entities
    void check_collisions(Registry& registry);

private:
    // Helper function to check collision between two entities
    bool check_collision(const Entity& entity1, const Entity& entity2, const Registry& registry);

    // Helper function to create a Collision component
    void create_collision(Entity entity1, Entity entity2, Registry& registry);
};