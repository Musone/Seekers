#pragma once

#include <GLFW/glfw3.h>
#include <utils/Transform.hpp>
#include <systems/AudioSystem.hpp>

#include <app/EntityFactory.hpp>
#include "ecs/Registry.hpp"
#include "globals/Globals.h"
#include "utils/Common.hpp"
#include "utils/Timer.h"

namespace InputManager {
    inline void on_key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Registry& registry = Registry::get_instance();
        Motion& player_motion = registry.motions.get(registry.player);
        AudioSystem& audio_system = AudioSystem::get_instance();

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_Z) {
                Globals::is_3d_mode = !Globals::is_3d_mode;
            }
            if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D) {
                audio_system.play_footstep();
            }
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
                audio_system.play_dodge();
                if (!registry.in_dodges.has(registry.player)) {
                    registry.in_dodges.emplace(registry.player, player_motion.position, player_motion.position + Common::normalize(player_motion.velocity) * Globals::dodgeMoveMag, Globals::timer.GetTime(), Globals::dodgeDuration);
                }
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
            if (!registry.input_state.w_down && !registry.input_state.s_down &&
                !registry.input_state.a_down && !registry.input_state.d_down) {
                audio_system.stop_footstep(-1);
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
            }
        }
    }

    inline void on_mouse_button_pressed(GLFWwindow* window, int button, int action, int mods) {
        Registry& registry = Registry::get_instance();
        Attacker& player_attacker = registry.attackers.get(registry.player);
        WeaponStats& weapon_stats = registry.weapon_stats.get(player_attacker.weapon_id);
        AudioSystem& audio_system = AudioSystem::get_instance();

        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (!registry.attack_cooldowns.has(registry.player)) {
                    EntityFactory::create_projectile(registry.motions.get(registry.player), player_attacker, weapon_stats, TEAM_ID::FRIENDLY);
                    audio_system.play_attack();
                    registry.attack_cooldowns.emplace(registry.player, weapon_stats.attack_cooldown);
                }
            }
        }
    }

    inline void on_mouse_move(GLFWwindow* window, double x, double y) {
        Registry& registry = Registry::get_instance();
        if (Globals::is_3d_mode) {
            auto& player_motion = registry.motions.get(registry.player);
            player_motion.angle = (WINDOW_WIDTH / 2 - x) / (WINDOW_WIDTH / 2);
        }
        registry.input_state.mouse_pos = glm::vec2(x, WINDOW_HEIGHT - y);
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
        if (Globals::is_3d_mode) {
            // Shoot straight when in 3d-mode
            const auto& temp2 = Transform::create_rotation_matrix({ 0, 0, player_motion.angle }) * glm::vec4(0, 1, 0, 1);
            player_attacker.aim = { temp2.x, temp2.y };
        } else {
            player_attacker.aim = Common::normalize(input_state.mouse_pos - glm::vec2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));
            temp = Transform::create_rotation_matrix({0, 0, player_motion.angle}) * glm::vec4(player_attacker.aim, 0, 1);
            player_attacker.aim = { temp.x, temp.y };
        }
    }
};
