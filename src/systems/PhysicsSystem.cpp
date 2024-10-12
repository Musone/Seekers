#include "PhysicsSystem.hpp"
#include "../utils/Log.hpp"
#include "../utils/Common.hpp"

void PhysicsSystem::step(float elapsed_ms)
{
    // update the motions
    for (Motion& motion : registry.motions.components) {
        glm::vec2 drag = -Common::normalize(motion.velocity) * motion.drag;
        motion.velocity += (motion.acceleration + drag) * elapsed_ms;
        motion.position += motion.velocity * elapsed_ms;
        motion.angle += motion.rotation_velocity * elapsed_ms;
    }
}
