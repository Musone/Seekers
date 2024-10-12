#include "PhysicsSystem.hpp"
#include "../utils/Log.hpp"

void PhysicsSystem::step(Registry& registry, float elapsed_ms)
{
    // TODO: Implement physics update logic
    // 1. Iterate through entities with Motion components
    // 2. Update their position and velocity based on elapsed time
    // 3. Apply drag if applicable
}

void PhysicsSystem::update_motion(Entity entity, Registry& registry, float elapsed_ms)
{
    // TODO: Implement motion update for a single entity
    // 1. Update velocity based on acceleration
    // 2. Update position based on velocity
    // 3. Apply rotation if applicable
}

void PhysicsSystem::apply_drag(Motion& motion, float elapsed_ms)
{
    // TODO: Implement drag application
    // Reduce velocity based on the drag coefficient
}