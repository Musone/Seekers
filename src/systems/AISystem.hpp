#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include "utils/Common.hpp"

namespace AISystem
{
    inline void update_target_position(AIComponent& ai) {
        for (int i = 0; i < ai.patrol_points.size(); i++) {
            if (ai.patrol_points[i] == ai.target_position) {
                ai.target_position = ai.patrol_points[(i+1) % ai.patrol_points.size()];
                return;
            }
        }
        ai.target_position = ai.patrol_points[0];
    }

    inline void AI_patrol_step(Entity& e) {
        Registry& registry = Registry::get_instance();
        Motion& motion = registry.motions.get(e);
        AIComponent& ai = registry.ais.get(e);

        if (glm::length(motion.position - ai.target_position) < Globals::ai_distance_epsilon) {
            update_target_position(ai);
        }
        glm::vec2 dir = Common::normalize(ai.target_position - motion.position);
        motion.velocity = registry.locomotion_stats.get(e).movement_speed * dir;
    }

    inline void AI_step() {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.ais.entities) {
            AIComponent& ai = registry.ais.get(e);
            if (ai.current_state == AI_STATE::PATROL) {
                AI_patrol_step(e);
            }
        }
    }
};