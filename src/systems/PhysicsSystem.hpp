#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include "utils/Common.hpp"

namespace PhysicsSystem
{
    inline void step(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& entity : registry.motions.entities) {
            Motion& motion = registry.motions.get(entity);
            if (!registry.in_dodges.has(entity)) {
                glm::vec2 drag = -Common::normalize(motion.velocity) * motion.drag;
                motion.velocity += (motion.acceleration + drag) * (elapsed_ms / 1000.0f);
                motion.position += motion.velocity * (elapsed_ms / 1000.0f);
            }
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

    inline void update_interpolations() {
        Registry& registry = Registry::get_instance();

        for (Entity& entity : registry.in_dodges.entities) {
            InDodge& indodge = registry.in_dodges.get(entity);
            float y1 = indodge.source.x;
            float y2 = indodge.destination.x;
            float x1 = indodge.origin_time / 1000000.0f;
            float x2 = indodge.origin_time / 1000000.0f + indodge.duration;
            float x = Globals::timer.GetTime() / 1000000.0f;
            float x_pos = y1 + (x - x1) * (y2 - y1) / (x2 - x1);

            y1 = indodge.source.y;
            y2 = indodge.destination.y;
            float y_pos = y1 + (x - x1) * (y2 - y1) / (x2 - x1);

            registry.motions.get(entity).position = glm::vec2(x_pos, y_pos);

            if (x - x1 > indodge.duration) {
                registry.in_dodges.remove(entity);
            }
        }
    }
};