#pragma once

#include <GLFW/glfw3.h>
#include <utils/Transform.hpp>
#include <systems/GameplaySystem.hpp>
#include <systems/TutorialSystem.hpp>

#include "MapManager.hpp"
#include "ecs/Registry.hpp"
#include "globals/Globals.h"
#include "utils/Common.hpp"
#include "systems/SaveLoadManager.hpp"

namespace InputManager {
    inline void on_key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& player_motion = registry.motions.get(registry.player);

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_Z) {
                Globals::is_3d_mode = !Globals::is_3d_mode;
            }
            if (key == GLFW_KEY_W) {
                registry.input_state.w_down = true;
                TutorialSystem::pass_movements();
            }
            if (key == GLFW_KEY_S) {
                registry.input_state.s_down = true;
                TutorialSystem::pass_movements();
            }
            if (key == GLFW_KEY_A) {
                registry.input_state.a_down = true;
                TutorialSystem::pass_movements();
            }
            if (key == GLFW_KEY_D) {
                registry.input_state.d_down = true;
                TutorialSystem::pass_movements();
            }
            if (key == GLFW_KEY_Q) {
                player_motion.rotation_velocity += Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_E) {
                player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            }
            if (key == GLFW_KEY_SPACE) {
                GameplaySystem::dodge(registry.player);
                TutorialSystem::pass_dodge();
            }
            if (key == GLFW_KEY_P) {
                TutorialSystem::skip_tutorial();
            }

            if (key == GLFW_KEY_G) {
                MapManager::get_instance().enter_dungeon_flag = true;
            }
            if (key == GLFW_KEY_R) {
                MapManager::get_instance().return_open_world_flag = true;
            }

            if (key == GLFW_KEY_F5) {
                SaveLoadManager::get_instance().save_game("quicksave");
            }
            if (key == GLFW_KEY_F6) {
                SaveLoadManager::get_instance().load_game("quicksave");
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

    inline void on_mouse_button_pressed(GLFWwindow* window, int button, int action, int mods) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Attacker& player_attacker = registry.attackers.get(registry.player);
        Weapon& weapon_stats = registry.weapons.get(player_attacker.weapon);

        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (!registry.attack_cooldowns.has(registry.player)) {
                    GameplaySystem::attack(registry.player);
                }
                TutorialSystem::pass_attack();
            }
        }
    }

    inline void on_mouse_move(GLFWwindow* window, double x, double y) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        if (Globals::is_3d_mode) {
            if (!registry.death_cooldowns.has(registry.player)) {
                auto& player_motion = registry.motions.get(registry.player);
                player_motion.angle = (WINDOW_WIDTH / 2 - x) / (WINDOW_WIDTH / 2);
            }
        }
        registry.input_state.mouse_pos = glm::vec2(x, WINDOW_HEIGHT - y);
        TutorialSystem::pass_aim();
    }

    // handle inputs that need updates every frame which should be called in World::step()
    inline void handle_inputs_per_frame() {
        Registry& registry = MapManager::get_instance().get_active_registry();
        InputState& input_state = registry.input_state;
        LocomotionStats& player_stats = registry.locomotion_stats.get(registry.player);
        Motion& player_motion = registry.motions.get(registry.player);
        Attacker& player_attacker = registry.attackers.get(registry.player);

        // movement
        glm::vec2 move_dir = glm::vec2(0.f, 0.f);
        if (input_state.w_down) {move_dir.x += 1.f;}
        if (input_state.s_down) {move_dir.x -= 1.f;}
        if (input_state.a_down) {move_dir.y += 1.f;}
        if (input_state.d_down) {move_dir.y -= 1.f;}

        move_dir = Common::normalize(move_dir);
        move_dir *= player_stats.movement_speed;
        glm::vec4 temp = Transform::create_rotation_matrix({ 0, 0, player_motion.angle }) * glm::vec4(move_dir, 0, 1);
        if (!registry.stagger_cooldowns.has(registry.player)) {
            player_motion.velocity = { temp.x, temp.y };
        } else {
            player_motion.velocity = 0.25f * glm::vec2(temp.x, temp.y);
        }

        // update aim
        if (Globals::is_3d_mode) {
            // Shoot straight when in 3d-mode
            const auto& temp2 = Transform::create_rotation_matrix({ 0, 0, player_motion.angle }) * glm::vec4(1, 0, 0, 0);
            player_attacker.aim = { temp2.x, temp2.y };
        } else {
            player_attacker.aim = Common::normalize(input_state.mouse_pos - glm::vec2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));
            temp = Transform::create_rotation_matrix({0, 0, player_motion.angle}) * glm::vec4(player_attacker.aim, 0, 1);
            player_attacker.aim = { temp.x, temp.y };
        }
    }
};
