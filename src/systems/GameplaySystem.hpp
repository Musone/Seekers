#pragma once

#include <globals/Globals.h>
#include <app/World.h>
#include "../ecs/Registry.hpp"
#include <app/EntityFactory.hpp>

namespace GameplaySystem {
    inline void update_cooldowns(float elapsed_ms) {
        Registry& registry = MapManager::get_instance().get_active_registry();

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
        Registry& registry = MapManager::get_instance().get_active_registry();

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
        Registry& registry = MapManager::get_instance().get_active_registry();

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
        Registry& registry = MapManager::get_instance().get_active_registry();

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

        for (Entity& e : registry.light_sources.entities) {
            auto& light_pos = registry.light_sources.get(e).pos;
            float distance_camera = glm::distance(registry.camera_pos, glm::vec2(light_pos.x, light_pos.y));
            if (distance_camera < Globals::static_render_distance) {
                registry.near_cameras.emplace(e);
            }
        }
    }

    inline void deplete_energy(const Entity& e, const float amount) {
        Registry& registry = MapManager::get_instance().get_active_registry();

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
        Registry& registry = MapManager::get_instance().get_active_registry();
        AudioSystem& audio = AudioSystem::get_instance();

        LocomotionStats& locomotion = registry.locomotion_stats.get(e);

        if (registry.attack_cooldowns.has(e) || registry.stagger_cooldowns.has(e) || registry.death_cooldowns.has(e) || locomotion.energy <= 0) return;

        Motion& motion = registry.motions.get(e);
        Attacker& attacker = registry.attackers.get(e);
        Weapon& weapon = registry.weapons.get(attacker.weapon);

        EntityFactory::create_projectile(registry, motion, attacker, weapon, registry.teams.get(e).team_id);
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
        Registry& registry = MapManager::get_instance().get_active_registry();
        AudioSystem& audio = AudioSystem::get_instance();

        LocomotionStats& locomotion = registry.locomotion_stats.get(e);

        if (registry.in_dodges.has(e) || locomotion.energy <= 0) return;

        Motion& motion = registry.motions.get(e);

        glm::vec2 dodge_target_pos;
        if (glm::length(motion.velocity) < 0.00001) {
            dodge_target_pos = motion.position + -glm::vec2(cos(motion.angle), sin(motion.angle)) * Globals::dodgeMoveMag;
        } else {
            dodge_target_pos = motion.position + Common::normalize(motion.velocity) * Globals::dodgeMoveMag;
        }
        registry.in_dodges.emplace(e, motion.position, dodge_target_pos, Globals::timer.GetTime(), Globals::dodgeDuration);
        deplete_energy(e, Globals::dodge_energy_cost);

        float distance_from_camera = glm::distance(registry.camera_pos, motion.position);
        audio.play_dodge(distance_from_camera);
    }

    inline void consume_estus() {
        Registry& registry = MapManager::get_instance().get_active_registry();
        std::vector<Entity>& esti = registry.inventory.estus;

        if (esti.size() <= 0 || registry.attack_cooldowns.has(registry.player) || registry.stagger_cooldowns.has(registry.player) || registry.death_cooldowns.has(registry.player)) return;

        LocomotionStats& loco =  registry.locomotion_stats.get(registry.player);
        loco.health = fmin(loco.health + registry.estus.get(esti[0]).heal_amount, loco.max_health);
        registry.remove_all_components_of(esti[0]);
        esti.erase(esti.begin());
    }

    inline void rest() {
        Registry& registry = MapManager::get_instance().get_active_registry();

        if (registry.in_rests.has(registry.player)) {
            registry.in_rests.remove(registry.player);
            Globals::is_getting_up = true;
            return;
        }

        LocomotionStats& loco = registry.locomotion_stats.get(registry.player);
        loco.health = loco.max_health;
        loco.energy = loco.max_energy;
        loco.poise = loco.max_poise;
        while (registry.inventory.estus.size() < 3) {
            Entity e = Entity();
            registry.inventory.estus.push_back(e);
            auto& estus = registry.estus.emplace(e);
            estus.heal_amount = 120.0f;
        }

        registry.input_state.w_down = false;
        registry.input_state.a_down = false;
        registry.input_state.s_down = false;
        registry.input_state.d_down = false;

        registry.in_rests.emplace(registry.player);
        // maybe respawn enemies here
        // save here or in interaction
    }

    inline void lock_on_target() {
        Registry& registry = MapManager::get_instance().get_active_registry();
        GLFWwindow* window = static_cast<GLFWwindow*>(Globals::ptr_window);

        if (!registry.locked_target.is_active) {
            double ypos;
            glfwGetCursorPos(window, nullptr, &ypos);
            double xpos = WINDOW_WIDTH * (1 - registry.motions.get(registry.player).angle) / 2;
            glfwSetCursorPos(window, xpos, ypos);
            return;
        }

        float min_angle = std::numeric_limits<float>::max();
        auto& player_motion = registry.motions.get(registry.player);
        for (Entity& e : registry.near_players.entities) {
            if (!registry.enemies.has(e)) continue;
            auto& motion = registry.motions.get(e);
            if (glm::distance(player_motion.position, motion.position) > Globals::lock_target_range || registry.death_cooldowns.has(e)) continue;
            float angle = Common::get_angle_between_item_and_player_view(motion.position, player_motion.position, player_motion.angle);
            if (angle < min_angle) {
                registry.locked_target.target = e;
                min_angle = angle;
            }
        }
        if (min_angle == std::numeric_limits<float>::max()) { // no target was found to lock on
            registry.locked_target.is_active = false;
            double ypos;
            glfwGetCursorPos(window, nullptr, &ypos);
            double xpos = WINDOW_WIDTH * (1 - registry.motions.get(registry.player).angle) / 2;
            glfwSetCursorPos(window, xpos, ypos);
        }
    }

    inline void switch_target(float delta_mouse_x) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        if (!registry.locked_target.is_active) return;

        float min_angle = std::numeric_limits<float>::max();
        auto& player_motion = registry.motions.get(registry.player);
        for (Entity& e : registry.near_players.entities) {
            if (!registry.enemies.has(e)) continue;
            auto& motion = registry.motions.get(e);
            if (glm::distance(player_motion.position, motion.position) > Globals::lock_target_range || registry.death_cooldowns.has(e)) continue;
            float angle = Common::get_angle_between_item_and_player_view(motion.position, player_motion.position, player_motion.angle - delta_mouse_x);
            if (angle < min_angle) {
                registry.locked_target.target = e;
                min_angle = angle;
            }
        }
        if (min_angle == std::numeric_limits<float>::max()) { // no target was found to lock on
            registry.locked_target.is_active = false;
        }
    }
};