#pragma once

#include <GLFW/glfw3.h>
#include <utils/Transform.hpp>
#include <systems/GameplaySystem.hpp>
#include <systems/InteractionSystem.hpp>
#include <systems/TutorialSystem.hpp>

#include "MapManager.hpp"
#include "ecs/Registry.hpp"
#include "globals/Globals.h"
#include "utils/Common.hpp"
#include "systems/SaveLoadSystem.hpp"

namespace InputManager {
    inline void on_key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& player_motion = registry.motions.get(registry.player);

        if (Globals::is_getting_up) return;

        if (action == GLFW_PRESS && key == GLFW_KEY_F) {
            InteractionSystem::interact();
        }

        if (Globals::is_getting_up || registry.in_rests.has(registry.player)) return;

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
            // if (key == GLFW_KEY_Q) {
            //     player_motion.rotation_velocity += Globals::cameraRotationSpeed;
            // }
            // if (key == GLFW_KEY_E) {
            //     player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            // }
            if (key == GLFW_KEY_SPACE) {
                GameplaySystem::dodge(registry.player);
                TutorialSystem::pass_dodge();
            }
            if (key == GLFW_KEY_P) {
                TutorialSystem::skip_tutorial();
            }
            if (key == GLFW_KEY_1) {
                GameplaySystem::consume_estus();
            }

            // if (key == GLFW_KEY_G) {
            //     MapManager::get_instance().enter_dungeon_flag = true;
            // }
            // if (key == GLFW_KEY_R) {
            //     MapManager::get_instance().return_open_world_flag = true;
            // }

            if (key == GLFW_KEY_F5) {
                // Simple save with timestamp as name
                std::string timestamp = std::to_string(std::time(nullptr));
                if (SaveLoadSystem::save_game(registry, "Save_" + timestamp)) {
                    std::cout << "Game saved successfully" << std::endl;
                } else {
                    std::cout << "Failed to save game" << std::endl;
                }
            }
            if (key == GLFW_KEY_F6) {
                // Load latest save
                if (SaveLoadSystem::load_game(registry)) {
                    std::cout << "Game loaded successfully" << std::endl;
                } else {
                    std::cout << "Failed to load game" << std::endl;
                }
            }
            if (key == GLFW_KEY_F7) {
                // List all save files
                auto& save_system = SaveLoadSystem::get_instance();
                auto slots = save_system.list_save_slots();

                if (slots.empty()) {
                    std::cout << "No save files found" << std::endl;
                } else {
                    std::cout << "\n=== Save Files ===" << std::endl;
                    for (const auto& slot : slots) {
                        std::string time_str = std::ctime(&slot.timestamp);
                        time_str = time_str.substr(0, time_str.length() - 1); // Remove newline
                        std::cout << "ID: " << slot.id
                                 << " | Name: " << slot.name
                                 << " | File: " << slot.filename
                                 << " | Saved: " << time_str
                                 << std::endl;
                    }
                    std::cout << "===============\n" << std::endl;
                }
            }

            // if (key == GLFW_KEY_G) {
            //     MapManager::get_instance().enter_dungeon_flag = true;
            // }
            // if (key == GLFW_KEY_R) {
            //     MapManager::get_instance().return_open_world_flag = true;
            // }
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
            // if (key == GLFW_KEY_Q) {
            //     player_motion.rotation_velocity -= Globals::cameraRotationSpeed;
            // }
            // if (key == GLFW_KEY_E) {
            //     player_motion.rotation_velocity += Globals::cameraRotationSpeed;
            // }
        }
    }

    inline void on_mouse_button_pressed(GLFWwindow* window, int button, int action, int mods) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        if (Globals::is_getting_up || registry.in_rests.has(registry.player)) return;

        Attacker& player_attacker = registry.attackers.get(registry.player);
        Weapon& weapon_stats = registry.weapons.get(player_attacker.weapon);

        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                GameplaySystem::attack(registry.player);
                TutorialSystem::pass_attack();
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                registry.locked_target.is_active = !registry.locked_target.is_active;
                GameplaySystem::lock_on_target();
            }
        }
    }

    inline void on_mouse_move(GLFWwindow* window, double x, double y) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        if (Globals::is_getting_up || registry.in_rests.has(registry.player)) return;

        if (Globals::is_3d_mode) {
            if (!registry.death_cooldowns.has(registry.player)) {
                if (!registry.locked_target.is_active) {
                    auto& player_motion = registry.motions.get(registry.player);
                    player_motion.angle = (WINDOW_WIDTH / 2 - x) / (WINDOW_WIDTH / 2);
                } else {
                    GameplaySystem::switch_target((x - registry.input_state.mouse_pos.x) / (WINDOW_WIDTH / 2));
                }
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

        if (registry.locked_target.is_active) {
            glm::vec2 pos = registry.motions.get(registry.locked_target.target).position;
            glm::vec2 dir = pos - player_motion.position;
            player_motion.angle = atan2(dir.y, dir.x);
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
