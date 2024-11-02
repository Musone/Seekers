#pragma once

#include <globals/Globals.h>
#include <app/World.h>
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

        for (Entity& e : registry.stagger_cooldowns.entities) {
            auto& stagger_cooldown = registry.stagger_cooldowns.get(e);
            stagger_cooldown.timer -= elapsed_ms / 1000.0f;
            if (stagger_cooldown.timer <= 0) {
                registry.attack_cooldowns.remove(e);
            }
        }

        for (Entity& e : registry.death_cooldowns.entities) {
            auto& death_cooldown = registry.death_cooldowns.get(e);
            death_cooldown.timer -= elapsed_ms / 1000.0f;
            if (death_cooldown.timer <= 0) {
                registry.remove_all_components_of(e);
                if (registry.player == e) {World::restart_game();}
            }
        }
    }

    inline void update_regen_stats(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.near_players.entities) {
            if (registry.locomotion_stats.has(e)) {
                auto& loco = registry.locomotion_stats.get(e);

                loco.energy += Globals::energy_regen_rate * elapsed_ms / 1000.0f;
                loco.energy = fmin(loco.energy, loco.max_energy);
                loco.poise += Globals::poise_regen_multiplier * loco.max_poise * elapsed_ms / 1000.0f;
                loco.poise = fmin(loco.poise, loco.max_poise);
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

    inline void update_near_player_camera() {
        Registry& registry = Registry::get_instance();

        registry.near_players.clear();
        registry.near_cameras.clear();

        auto& player_motion = registry.motions.get(registry.player);
        for (Entity& e : registry.motions.entities) {
            auto& motion = registry.motions.get(e);

            float distance_player = glm::distance(player_motion.position, motion.position);
            if (distance_player < Globals::update_distance) {
                registry.near_players.emplace(e);
            }

            float distance_camera = glm::distance(registry.camera_pos, motion.position);
            if (distance_camera < Globals::update_distance) {
                registry.near_cameras.emplace(e);
            }
        }
    }
};