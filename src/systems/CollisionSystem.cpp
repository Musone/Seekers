#include "CollisionSystem.hpp"
#include "../utils/Log.hpp"

void CollisionSystem::check_collisions(Registry& registry)
{
    // TODO: Implement collision detection logic
    // 1. Iterate through entities with Motion components
    // 2. Check for collisions between entities
    // 3. Create Collision components for colliding entities
}

bool CollisionSystem::check_collision(const Entity& entity1, const Entity& entity2, const Registry& registry)
{
    // TODO: Implement collision detection between two entities
    // Use circular collision detection or AABB (Axis-Aligned Bounding Box) based on our needs
    return false;
}

void CollisionSystem::create_collision(Entity entity1, Entity entity2, Registry& registry)
{
    // TODO: Create Collision components for the colliding entities
    // registry.emplace<Collision>(entity1, entity2);
    // registry.emplace<Collision>(entity2, entity1);
}