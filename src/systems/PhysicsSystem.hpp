#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"

class PhysicsSystem
{
public:
    PhysicsSystem() = default;
    ~PhysicsSystem() = default;

    // Update physics for all entities with Motion components
    void step(Registry& registry, float elapsed_ms);

private:
    // Update position and velocity for a single entity
    void update_motion(Entity entity, Registry& registry, float elapsed_ms);

    // Apply drag to entity's motion
    void apply_drag(Motion& motion, float elapsed_ms);
};