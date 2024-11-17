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
                SaveLoadManager& save_manager = SaveLoadManager::get_instance();
                
                // Debug: List all current save slots
                auto slots = save_manager.list_save_slots();
                std::cout << "Current save slots:" << std::endl;
                for (const auto& slot : slots) {
                    std::cout << "Slot " << slot.id << ": " << slot.name 
                             << " (saved at: " << std::ctime(&slot.timestamp) << ")" << std::endl;
                }
                
                // Create a new slot with timestamp in name for testing
                auto time_str = std::to_string(std::time(nullptr));
                SaveSlot new_slot = save_manager.create_new_slot("Save_" + time_str);
                save_manager.save_game_to_slot(new_slot);
            }
            if (key == GLFW_KEY_F6) {
                SaveLoadManager& save_manager = SaveLoadManager::get_instance();
                auto slots = save_manager.list_save_slots();
                
                if (!slots.empty()) {
                    // Try to load the most recent save
                    std::cout << "Loading most recent save: " << slots.back().name << std::endl;
                    save_manager.load_game_from_slot(slots.back());
                } else {
                    std::cout << "No save slots available" << std::endl;
                }
            }
            if (key == GLFW_KEY_F9) {
                SaveLoadManager& save_manager = SaveLoadManager::get_instance();
                // Create a new game slot
                SaveSlot new_slot = save_manager.create_new_slot("New Game");
                if (save_manager.create_new_game("New Game")) {
                    std::cout << "Created new game in slot: " << new_slot.name << std::endl;
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
