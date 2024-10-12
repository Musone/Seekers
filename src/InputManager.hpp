#pragma once

#include <stdexcept>
#include <GLFW/glfw3.h>

#include "ecs/Registry.hpp"
#include "globals/Globals.h"
#include "utils/Common.hpp"

namespace InputManager {
    inline void on_key_pressed(int key, int scancode, int action, int mods) {
        Registry& registry = Registry::get_instance();
        LocomotionStats& player_stats = registry.locomotion_stats.get(registry.player);
        Motion& player_motion = registry.motions.get(registry.player);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_W) {
                player_motion.velocity.x += player_stats.movement_speed * cos(player_motion.angle);
                player_motion.velocity.y += player_stats.movement_speed * sin(player_motion.angle);
            }
            if (key == GLFW_KEY_S) {
                player_motion.velocity.x -= player_stats.movement_speed * cos(player_motion.angle);
                player_motion.velocity.y -= player_stats.movement_speed * sin(player_motion.angle);
            }
            if (key == GLFW_KEY_A) {
                player_motion.velocity.x -= player_stats.movement_speed * cos(player_motion.angle + M_PI / 2.f);
                player_motion.velocity.y -= player_stats.movement_speed * sin(player_motion.angle + M_PI / 2.f);
            }
            if (key == GLFW_KEY_D) {
                player_motion.velocity.x += player_stats.movement_speed * cos(player_motion.angle + M_PI / 2.f);
                player_motion.velocity.y += player_stats.movement_speed * sin(player_motion.angle + M_PI / 2.f);
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
                for (RotateWithPlayer& rotate_with_player : registry.rotate_with_players.components) {
                    rotate_with_player.motion.rotation_velocity += Globals::cameraRotationSpeed;
                }
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
                for (RotateWithPlayer& rotate_with_player : registry.rotate_with_players.components) {
                    rotate_with_player.motion.rotation_velocity -= Globals::cameraRotationSpeed;
                }
            }
            if (key == GLFW_KEY_SPACE) {
                player_motion.position += Common::normalize(player_motion.velocity) * Globals::dodgeMoveMag;
            }
        }
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_W) {
                player_motion.velocity.x -= player_stats.movement_speed * cos(player_motion.angle);
                player_motion.velocity.y -= player_stats.movement_speed * sin(player_motion.angle);
            }
            if (key == GLFW_KEY_S) {
                player_motion.velocity.x += player_stats.movement_speed * cos(player_motion.angle);
                player_motion.velocity.y += player_stats.movement_speed * sin(player_motion.angle);
            }
            if (key == GLFW_KEY_A) {
                player_motion.velocity.x += player_stats.movement_speed * cos(player_motion.angle + M_PI / 2.f);
                player_motion.velocity.y += player_stats.movement_speed * sin(player_motion.angle + M_PI / 2.f);
            }
            if (key == GLFW_KEY_D) {
                player_motion.velocity.x -= player_stats.movement_speed * cos(player_motion.angle + M_PI / 2.f);
                player_motion.velocity.y -= player_stats.movement_speed * sin(player_motion.angle + M_PI / 2.f);
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
                for (RotateWithPlayer& rotate_with_player : registry.rotate_with_players.components) {
                    rotate_with_player.motion.rotation_velocity -= Globals::cameraRotationSpeed;
                }
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
                for (RotateWithPlayer& rotate_with_player : registry.rotate_with_players.components) {
                    rotate_with_player.motion.rotation_velocity += Globals::cameraRotationSpeed;
                }
            }
        }
    }

    inline void on_mouse_button_pressed(int button, int action, int mods) {
        Registry& registry = Registry::get_instance();
        Attacker& player_attacker = registry.attackers.get(registry.player);
        WeaponStats& weapon_stats = registry.weapon_stats.get(player_attacker.weapon_id);

        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (!registry.attack_cooldowns.has(registry.player)) {
                    auto projectile = Entity();

                    Motion& motion = registry.motions.emplace(projectile);
                    motion.position = registry.motions.get(registry.player).position;
                    motion.angle = atan2(player_attacker.aim.y, player_attacker.aim.x);
                    motion.velocity = player_attacker.aim * weapon_stats.proj_speed;

                    ProjectileStats& projectile_stats = registry.projectile_stats.emplace(projectile);
                    projectile_stats.damage = weapon_stats.damage;
                    projectile_stats.range_remaining = weapon_stats.range;

                    Team& team = registry.teams.emplace(projectile);
                    team.team_id = registry.teams.get(registry.player).team_id;

                    registry.attack_cooldowns.emplace(registry.player, weapon_stats.attack_cooldown);
                }
            }
        }
    }

    inline void on_mouse_move(int x, int y) {
        Registry& registry = Registry::get_instance();
        Attacker& player_attacker = registry.attackers.get(registry.player);
        Motion& player_motion = registry.motions.get(registry.player);

        player_attacker.aim = Common::normalize(player_motion.position - glm::vec2(x, y));
    }
}
