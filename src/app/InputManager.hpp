#pragma once

#include <stdexcept>
#include <GLFW/glfw3.h>
#include <utils/Transform.hpp>

#include <app/EntityFactory.hpp>
#include "ecs/Registry.hpp"
#include "globals/Globals.h"
#include "utils/Common.hpp"

namespace InputManager {
    inline void on_key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Registry& registry = Registry::get_instance();
        Motion& player_motion = registry.motions.get(registry.player);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_W) {
                registry.input_state.w_down = true;
            }
            if (key == GLFW_KEY_S) {
                registry.input_state.s_down = true;
            }
            if (key == GLFW_KEY_A) {
                registry.input_state.a_down = true;
            }
            if (key == GLFW_KEY_D) {
                registry.input_state.d_down = true;
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_SPACE) {
                player_motion.position += Common::normalize(player_motion.velocity) * Globals::dodgeMoveMag;
            }
        }
        if (action == GLFW_RELEASE) {
            if (key == GLFW_KEY_W) {
                registry.input_state.w_down = false;
            }
            if (key == GLFW_KEY_S) {
                registry.input_state.s_down = false;
            }
            if (key == GLFW_KEY_A) {
                registry.input_state.a_down = false;
            }
            if (key == GLFW_KEY_D) {
                registry.input_state.d_down = false;
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
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
                    EntityFactory::create_projectile(registry.motions.get(registry.player).position, player_attacker, weapon_stats, TEAM_ID::FRIENDLY);

                    registry.attack_cooldowns.emplace(registry.player, weapon_stats.attack_cooldown);
                }
            }
        }
    }

    inline void on_mouse_move(GLFWwindow* window, float x, float y) {
        Registry& registry = Registry::get_instance();

        registry.input_state.mouse_pos = glm::vec2(x, y);
    }

    // handle inputs that need updates every frame which should be called in World::step()
    inline void handle_inputs_per_frame() {
        Registry& registry = Registry::get_instance();
        InputState& input_state = registry.input_state;
        LocomotionStats& player_stats = registry.locomotion_stats.get(registry.player);
        Motion& player_motion = registry.motions.get(registry.player);
        Attacker& player_attacker = registry.attackers.get(registry.player);

        // movement
        glm::vec2 move_dir = glm::vec2(0.f, 0.f);
        if (input_state.w_down) {move_dir.y += 1.f;}
        if (input_state.s_down) {move_dir.y -= 1.f;}
        if (input_state.a_down) {move_dir.x -= 1.f;}
        if (input_state.d_down) {move_dir.x += 1.f;}
        move_dir = Common::normalize(move_dir);
        move_dir *= player_stats.movement_speed;
        glm::vec4 temp = Transform::create_rotation_matrix({ 0, 0, player_motion.angle }) * glm::vec4(move_dir, 0, 1);
        player_motion.velocity = { temp.x, temp.y };

        // update aim
        player_attacker.aim = Common::normalize(player_motion.position - input_state.mouse_pos);
    }
};
