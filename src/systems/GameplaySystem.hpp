#pragma once

#include <globals/Globals.h>
#include <app/World.h>
#include "../ecs/Registry.hpp"
#include <app/EntityFactory.hpp>

namespace GameplaySystem {
    inline void update_cooldowns(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        std::vector<Entity> to_be_removed;

        to_be_removed.reserve(registry.attack_cooldowns.size());
        for (Entity& e : registry.attack_cooldowns.entities) {
            auto& attack_cooldown = registry.attack_cooldowns.get(e);
            attack_cooldown.timer -= elapsed_ms / 1000.0f;
            if (attack_cooldown.timer <= 0) {
                to_be_removed.push_back(e);
            }
        }
        for (Entity& e : to_be_removed) {
            registry.attack_cooldowns.remove(e);
        }

        to_be_removed.clear();
        to_be_removed.reserve(registry.energy_no_regen_cooldowns.size());
        for (Entity& e : registry.energy_no_regen_cooldowns.entities) {
            auto& energy_no_regen_cooldown = registry.energy_no_regen_cooldowns.get(e);
            energy_no_regen_cooldown.timer -= elapsed_ms / 1000.0f;
            if (energy_no_regen_cooldown.timer <= 0) {
                to_be_removed.push_back(e);
            }
        }
        for (Entity& e : to_be_removed) {
            registry.energy_no_regen_cooldowns.remove(e);
        }

        to_be_removed.clear();
        to_be_removed.reserve(registry.stagger_cooldowns.size());
        for (Entity& e : registry.stagger_cooldowns.entities) {
            auto& stagger_cooldown = registry.stagger_cooldowns.get(e);
            stagger_cooldown.timer -= elapsed_ms / 1000.0f;
            if (stagger_cooldown.timer <= 0) {
                to_be_removed.push_back(e);
            }
        }
        for (Entity& e : to_be_removed) {
            registry.stagger_cooldowns.remove(e);
        }

        to_be_removed.clear();
        to_be_removed.reserve(registry.death_cooldowns.entities.size());
        for (Entity& e : registry.death_cooldowns.entities) {
            auto& death_cooldown = registry.death_cooldowns.get(e);
            death_cooldown.timer -= elapsed_ms / 1000.0f;
            if (death_cooldown.timer <= 0) {
                to_be_removed.push_back(e);
            }
        }
        for (Entity& e : to_be_removed) {
            if (registry.player == e) {
                World::restart_game();
                return;
            }
            registry.remove_all_components_of(e);
        }
    }

    inline void update_regen_stats(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.near_players.entities) {
            if (registry.locomotion_stats.has(e)) {
                auto& loco = registry.locomotion_stats.get(e);

                if (!registry.energy_no_regen_cooldowns.has(e)) {
                    loco.energy += Globals::energy_regen_rate * elapsed_ms / 1000.0f;
                    loco.energy = fmin(loco.energy, loco.max_energy);
                }
                loco.poise += Globals::poise_regen_multiplier * loco.max_poise * elapsed_ms / 1000.0f;
                loco.poise = fmin(loco.poise, loco.max_poise);
            }
        }
    }

    inline void update_projectile_range(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        std::vector<Entity> to_be_removed;

        to_be_removed.reserve(registry.projectiles.size());
        for (Entity& e : registry.projectiles.entities) {
            Projectile& projectile = registry.projectiles.get(e);
            projectile.range_remaining -= (elapsed_ms / 1000) * glm::length(registry.motions.get(e).velocity);
            if (projectile.range_remaining <= 0) {
                to_be_removed.push_back(e);
            }
        }
        for (Entity& e : to_be_removed) {
            registry.remove_all_components_of(e);
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

    inline void deplete_energy(const Entity& e, const float amount) {
        Registry& registry = Registry::get_instance();

        LocomotionStats& locomotion = registry.locomotion_stats.get(e);
        locomotion.energy -= amount;
        if (locomotion.energy <= 0) {
            locomotion.energy = 0;
            if (registry.energy_no_regen_cooldowns.has(e)) {
                registry.energy_no_regen_cooldowns.get(e).timer = Globals::energy_no_regen_duration;
            } else {
                registry.energy_no_regen_cooldowns.emplace(e, Globals::energy_no_regen_duration);
            }
        }
    }

    inline void attack(Entity& e) {
        Registry& registry = Registry::get_instance();
        AudioSystem& audio = AudioSystem::get_instance();

        LocomotionStats& locomotion = registry.locomotion_stats.get(e);

        if (registry.attack_cooldowns.has(e) || registry.stagger_cooldowns.has(e) || registry.death_cooldowns.has(e) || locomotion.energy <= 0) return;

        Motion& motion = registry.motions.get(e);
        Attacker& attacker = registry.attackers.get(e);
        Weapon& weapon = registry.weapons.get(attacker.weapon_id);

        EntityFactory::create_projectile(motion, attacker, weapon, registry.teams.get(e).team_id);
        registry.attack_cooldowns.emplace(e, weapon.attack_cooldown);
        deplete_energy(e, weapon.attack_energy_cost);

        float distance_from_camera = glm::distance(registry.camera_pos, motion.position);
        if (weapon.type == WEAPON_TYPE::BOW) {
            audio.play_attack_bow(distance_from_camera);
        } else {
            audio.play_attack_sword(distance_from_camera);
        }
    }

    inline void dodge(Entity& e) {
        Registry& registry = Registry::get_instance();
        AudioSystem& audio = AudioSystem::get_instance();

        LocomotionStats& locomotion = registry.locomotion_stats.get(e);

        if (registry.in_dodges.has(e) || locomotion.energy <= 0) return;

        Motion& motion = registry.motions.get(e);

        // TODO: maybe jump back when 0 velocity
        registry.in_dodges.emplace(e, motion.position, motion.position + Common::normalize(motion.velocity) * Globals::dodgeMoveMag, Globals::timer.GetTime(), Globals::dodgeDuration);
        deplete_energy(e, Globals::dodge_energy_cost);

        float distance_from_camera = glm::distance(registry.camera_pos, motion.position);
        audio.play_dodge(distance_from_camera);
    }
};