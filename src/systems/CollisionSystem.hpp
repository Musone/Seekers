#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include <vector>
#include <string>

class CollisionSystem
{
public:
    CollisionSystem();
    ~CollisionSystem() = default;

    // Check for collisions between entities
    void check_collisions();

private:
    Registry& m_registry;

    // Helper function to check collision between two entities
    bool check_collision(const Motion& motion1, const Motion& motion2) const;

    // Helper function to create a Collision component
    void create_collision(const Entity& entity1, const Entity& entity2);

    // Broad-phase collision detection
    void get_nearby_entities(const Entity& entity) const;
};
