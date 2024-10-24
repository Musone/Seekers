#pragma once

#include <globals/Globals.h>

#include "../ecs/Registry.hpp"

namespace GameplaySystem {
    inline void update_cooldowns(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.attack_cooldowns.entities) {
            auto& attack_cooldown = registry.attack_cooldowns.get(e);
            attack_cooldown.timer -= elapsed_ms / 1000.0f;
            if (attack_cooldown.timer <= 0) {
                registry.attack_cooldowns.remove(e);
            }
        }
    }

    inline void update_projectile_range(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.projectile_stats.entities) {
            ProjectileStats& projectile_stats = registry.projectile_stats.get(e);
            projectile_stats.range_remaining -= (elapsed_ms / 1000) * glm::length(registry.motions.get(e).velocity);
            if (projectile_stats.range_remaining <= 0) {
                registry.remove_all_components_of(e);
            }
        }
    }

    inline void update_near_player() {
        Registry& registry = Registry::get_instance();

        registry.near_players.clear();

        auto& player_motion = registry.motions.get(registry.player);
        for (Entity& e : registry.motions.entities) {
            auto& motion = registry.motions.get(e);

            float distance = glm::distance(player_motion.position, motion.position);
            if (distance < Globals::update_distance) {
                registry.near_players.emplace(e);
            }
        }
    }
};