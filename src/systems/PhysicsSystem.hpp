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
    void step(float elapsed_ms);

private:
    Registry& registry = Registry::get_instance();
};