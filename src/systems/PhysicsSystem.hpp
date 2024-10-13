#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include "utils/Common.hpp"

namespace PhysicsSystem
{
    inline void step(float elapsed_ms) {
        Registry& registry = Registry::get_instance();
        
        // update the motions
        for (Motion& motion : registry.motions.components) {
            glm::vec2 drag = -Common::normalize(motion.velocity) * motion.drag;
            motion.velocity += (motion.acceleration + drag) * (elapsed_ms / 1000.0f);
            motion.position += motion.velocity * (elapsed_ms / 1000.0f);
            motion.angle += motion.rotation_velocity * (elapsed_ms / 1000.0f);
        }

        // update motion of follower entities
        for (Entity& e : registry.move_withs.entities) {
            Motion& follower = registry.motions.get(e);
            Motion& following = registry.motions.get(registry.move_withs.get(e).following_entity_id);
            follower.position = following.position + glm::vec2(1.0f, 0.0f);
        }
        for (Entity& e : registry.rotate_withs.entities) {
            Motion& follower = registry.motions.get(e);
            Motion& following = registry.motions.get(registry.rotate_withs.get(e).following_entity_id);
            follower.angle = following.angle;
        }
    }
};