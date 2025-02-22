#pragma once

#include <random>

#include "app/MapManager.hpp"
#include "GameplaySystem.hpp"
#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include "utils/Common.hpp"
#include "utils/PathFinder.hpp"

namespace AISystem
{
    inline void update_player_vision(float elapsed_ms) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        for (Entity& e : registry.ais.entities) {
            if (!registry.near_players.has(e)) {
                continue;
            }
            Motion& motion = registry.motions.get(e);

            glm::vec2 ai_position = get_grid_map_coordinates(motion);
            glm::vec2 target_position = get_grid_map_coordinates(registry.motions.get(registry.player));
            CollisionBounds ai_box = registry.collision_bounds.get(e);

            int collision_radius = 0;
            if (ai_box.type == ColliderType::Circle) {
                collision_radius = ai_box.circle.radius;
            }

            bool can_see_player = can_see(registry.grid_map.grid_boxes, ai_position.x, ai_position.y, collision_radius, target_position.x, target_position.y);
            if (registry.vision_to_players.has(e)) {
                auto& vision_to_player = registry.vision_to_players.get(e);
                if (can_see_player) {
                    vision_to_player.timer = 5.0f;
                } else {
                    vision_to_player.timer -= elapsed_ms / 1000.0f;
                    if (vision_to_player.timer <= 0) {
                        registry.attack_cooldowns.remove(e);
                    }
                }
            } else if (can_see_player) {
                registry.vision_to_players.emplace(e, 5.0f);
            }
        }
    }

//    I didn't know where to put this, so I put it here for now
    inline glm::vec2 rotate_vector(const glm::vec2& vec, float angle_degrees) {
        // Convert angle from degrees to radians
        float angle_radians = glm::radians(angle_degrees);

        // Calculate cosine and sine of the angle
        float cos_theta = cos(angle_radians);
        float sin_theta = sin(angle_radians);

        // Apply the rotation
        return {
                vec.x * cos_theta - vec.y * sin_theta,
                vec.x * sin_theta + vec.y * cos_theta
        };
    }
//    I didn't know where to put this, so I put it here for now
    inline bool is_gonna_collide(Entity& nearby_entity, Entity& ai_entity, glm::vec2 next_position) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        if (!registry.motions.has(nearby_entity)) {
            return false;
        }
        if (!registry.collision_bounds.has(nearby_entity)) {
            return false;
        }
        Motion& motion = registry.motions.get(nearby_entity);
        const CollisionBounds& nearby_bounds = registry.collision_bounds.get(nearby_entity);
        const CollisionBounds& ai_bounds = registry.collision_bounds.get(ai_entity);
        float distance = glm::length(next_position - motion.position);
        
        // Get combined radius for broad phase check
        float combined_radius = 0.0f;
        if (nearby_bounds.type == ColliderType::Circle && ai_bounds.type == ColliderType::Circle) {
            combined_radius = nearby_bounds.circle.radius + ai_bounds.circle.radius + 5.0f;
        } else {
            // For non-circle colliders, use conservative estimate
            if (nearby_bounds.type == ColliderType::Circle) {
                combined_radius += nearby_bounds.circle.radius;
            } else if (nearby_bounds.type == ColliderType::Wall) {
                combined_radius += glm::length(nearby_bounds.wall->aabb.max - nearby_bounds.wall->aabb.min) * 0.5f;
            } else if (nearby_bounds.type == ColliderType::Mesh) {
                combined_radius += nearby_bounds.mesh->bound_radius;
            }
            
            if (ai_bounds.type == ColliderType::Circle) {
                combined_radius += ai_bounds.circle.radius;
            } else if (ai_bounds.type == ColliderType::Wall) {
                combined_radius += glm::length(ai_bounds.wall->aabb.max - ai_bounds.wall->aabb.min) * 0.5f;
            } else if (ai_bounds.type == ColliderType::Mesh) {
                combined_radius += ai_bounds.mesh->bound_radius;
            }
            
            combined_radius += 5.0f;
        }
        
        if (distance < combined_radius) {
            return true;
        }
        return false;
    }

// This function should be refactored. It doesn't work properly right now, it needs a better algorithm.
    inline glm::vec2 get_ai_motion_velocity(Entity& e, AIComponent& ai) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        glm::vec2 dir = Common::normalize(ai.target_position - motion.position);
        glm::vec2 velocity = registry.locomotion_stats.get(e).movement_speed * dir;
        for (Entity& nearby_entity : registry.near_players.entities) {
            glm::vec2 next_position = motion.position + motion.velocity;
            if (is_gonna_collide(nearby_entity, e, next_position)) {
                for (int degree = 0; degree < 180; degree += 30) {
                    glm::vec2 rotated_dir = rotate_vector(dir, float(degree));
                    glm::vec2 rotated_velocity = registry.locomotion_stats.get(e).movement_speed * rotated_dir;
                    if (!is_gonna_collide(nearby_entity, e, motion.position + rotated_velocity)) {
                        velocity = rotated_velocity;
                        break;
                    }
                    rotated_dir = rotate_vector(dir, float(-1 * degree));
                    rotated_velocity = registry.locomotion_stats.get(e).movement_speed * rotated_dir;
                    if (!is_gonna_collide(nearby_entity, e, motion.position + rotated_velocity)) {
                        velocity = rotated_velocity;
                        break;
                    }
                }
                break;
            }
        }
        return velocity;
    }

    inline void update_patrol_target_position(AIComponent& ai) {
        for (int i = 0; i < ai.patrol_points.size(); i++) {
            if (ai.patrol_points[i] == ai.target_position) {
                ai.target_position = ai.patrol_points[(i+1) % ai.patrol_points.size()];
                return;
            }
        }
        ai.target_position = ai.patrol_points[0];
    }

    inline void update_chasing_target_position(AIComponent& ai) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        glm::vec2 player_position = registry.motions.get(registry.player).position;
        ai.target_position = player_position;
    }

    inline void AI_patrol_step(Entity& e) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        AIComponent& ai = registry.ais.get(e);

        if (glm::length(motion.position - ai.target_position) < Globals::ai_distance_epsilon) {
            update_patrol_target_position(ai);
        }
        glm::vec2 dir = Common::normalize(ai.target_position - motion.position);
        motion.velocity = registry.locomotion_stats.get(e).movement_speed * dir;
    }

    inline void AI_chase_step(Entity& e) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        AIComponent& ai = registry.ais.get(e);
        update_chasing_target_position(ai);

        glm::vec2 ai_position = get_grid_map_coordinates(motion);
        glm::vec2 target_position = get_grid_map_coordinates(registry.motions.get(registry.player));
        CollisionBounds& ai_box = registry.collision_bounds.get(e);
        
        // Get radius for pathfinding - assume circle collider
        float radius = 0.0f;
        if (ai_box.type == ColliderType::Circle) {
            radius = ai_box.circle.radius;
        }

        glm::vec2 next_position = get_next_point_of_path_to_player(
            registry.grid_map.grid_boxes, 
            ai_position.x, 
            ai_position.y, 
            radius
        );

        glm::vec2 new_position = get_position_from_grid_map_coordinates(next_position.x, next_position.y);

        glm::vec2 dir = Common::normalize(new_position - motion.position);
        motion.velocity = registry.locomotion_stats.get(e).movement_speed * dir;
    }

    inline void AI_attack_step(Entity& e) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> attack_dodge_dist(1, 50);

        if (attack_dodge_dist(gen) < 2) {
            GameplaySystem::dodge(e);
            return;
        }

        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        Attacker& attacker = registry.attackers.get(e);

        glm::vec2 player_position = registry.motions.get(registry.player).position;
        glm::vec2 direction = player_position - motion.position;

        attacker.aim = Common::normalize(player_position - motion.position);

        motion.angle = atan2(attacker.aim.y, attacker.aim.x);

        GameplaySystem::attack(e);
    }

    inline void AI_change_state(Entity& e) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        AIComponent& ai = registry.ais.get(e);
        glm::vec2 player_position = registry.motions.get(registry.player).position;
        if (glm::length(motion.position - player_position) < 20.0f) {
            ai.current_state = AI_STATE::ATTACK;
        } else if (glm::length(motion.position - player_position) < 30.0f) {
            ai.current_state = AI_STATE::CHASE;
        } else {
            ai.current_state = AI_STATE::PATROL;
        }
    }

    inline void AI_step() {
        Registry& registry = MapManager::get_instance().get_active_registry();

        for (Entity& e : registry.near_players.entities) {
            if (registry.ais.has(e) && !registry.death_cooldowns.has(e) && !registry.stagger_cooldowns.has(e)) {
                AIComponent &ai = registry.ais.get(e);
                if (ai.current_state == AI_STATE::PATROL) {
                    AI_patrol_step(e);
                } else if (ai.current_state == AI_STATE::CHASE) {
                    AI_chase_step(e);
                } else if (ai.current_state == AI_STATE::ATTACK) {
                    AI_chase_step(e);
                    AI_attack_step(e);
                }
                AI_change_state(e);
            }
        }
    }

    // boss AI stuff down here
    inline void boss_dodge(Entity& boss_entity, float dodge_ratio) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dodge_dist(0.0f, 1.0f);

        for (Entity& e : registry.projectiles.entities) {
            if (glm::distance(registry.motions.get(e).position, registry.motions.get(boss_entity).position) < 4.0f &&
                registry.teams.get(e).team_id == TEAM_ID::FRIENDLY) {
                if (dodge_dist(gen) <= dodge_ratio) {
                    GameplaySystem::dodge(boss_entity);
                    return;
                }
            }
        }
    }

    inline void boss_pick_combo(BossAI& comp) {
        // TODO: add Q-learning stuff here
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, comp.combos.size() - 1);
        comp.combo_index = dist(gen);
    }

    inline void boss_combo_step(Entity& e, BossAI& comp, float elapsed_ms) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& boss_motion = registry.motions.get(e);
        Motion& player_motion = registry.motions.get(registry.player);
        LocomotionStats& boss_loco = registry.locomotion_stats.get(e);

        comp.attack_delay_counter -= elapsed_ms / 1000.0f;
        // close the gap with the player if necessary
        if (glm::distance(boss_motion.position, player_motion.position) < comp.attack_range) {
            boss_motion.velocity = glm::vec2(0.0f);
        } else {
            boss_motion.velocity = glm::normalize(player_motion.position - boss_motion.position) * boss_loco.movement_speed;
            return;
        }

        if (comp.attack_delay_counter > 0.0f) return;

        BOSS_ATTACK_TYPE attack_type = comp.combos.at(comp.combo_index).attacks.at(comp.attack_index);
        if (!GameplaySystem::attack(e, 0.3, true, attack_type)) return;

        // update comp
        comp.attack_index++;
        if (comp.attack_index >= comp.combos.at(comp.combo_index).attacks.size()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> cooldown_dist(0.5f, 5.0f);
            std::uniform_real_distribution<float> move_multi_dist(-0.3f, 0.3f);
            comp.state = BOSS_STATE::COOLDOWN;
            comp.cooldown_delay_counter = cooldown_dist(gen);
            boss_motion.velocity = glm::vec2(-sin(boss_motion.angle), cos(boss_motion.angle)) * boss_loco.movement_speed * move_multi_dist(gen);
        } else {
            comp.attack_delay_counter = comp.combos.at(comp.combo_index).delays.at(comp.attack_index);
        }
    }

    inline void boss_cooldown(Entity& e, BossAI& comp, float elapsed_ms) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& boss_motion = registry.motions.get(e);
        glm::vec2 new_velocity = glm::vec2(-sin(boss_motion.angle), cos(boss_motion.angle)) * glm::length(boss_motion.velocity);
        boss_motion.velocity = glm::dot(boss_motion.velocity, new_velocity) > 0 ? new_velocity : -new_velocity;
        comp.cooldown_delay_counter -= elapsed_ms / 1000.0f;
        if (comp.cooldown_delay_counter <= 0.0f) {
            comp.state = BOSS_STATE::CHASE;
        }
        boss_dodge(e, comp.dodge_ratio);
    }

    inline void boss_chase(Entity& e, BossAI& comp) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& boss_motion = registry.motions.get(e);
        Motion& player_motion = registry.motions.get(registry.player);

        if (glm::distance(boss_motion.position, player_motion.position) < comp.attack_range) {
            comp.state = BOSS_STATE::IN_COMBO;
            boss_pick_combo(comp);
            comp.attack_index = 0;
            comp.attack_delay_counter = comp.combos.at(comp.combo_index).delays.at(0);
        } else {
            boss_motion.velocity = glm::normalize(player_motion.position - boss_motion.position) * registry.locomotion_stats.get(e).movement_speed;
            boss_dodge(e, comp.dodge_ratio);
        }
    }

    inline void boss_update_aim_angle(Entity& e) {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& motion = registry.motions.get(e);
        Attacker& attacker = registry.attackers.get(e);
        attacker.aim = glm::normalize(registry.motions.get(registry.player).position - motion.position);
        motion.angle = atan2(attacker.aim.y, attacker.aim.x);
    }

    inline void boss_AI_step(float elapsed_ms) {
        Registry& registry = MapManager::get_instance().get_active_registry();

        for (Entity& e : registry.boss_ais.entities) {
            boss_update_aim_angle(e);
            BossAI& comp = registry.boss_ais.get(e);
            if (comp.state == BOSS_STATE::IN_COMBO) {
                boss_combo_step(e, comp, elapsed_ms);
            } else if (comp.state == BOSS_STATE::COOLDOWN) {
                boss_cooldown(e, comp, elapsed_ms);
            } else if (comp.state == BOSS_STATE::CHASE) {
                boss_chase(e, comp);

            }
        }
    }
};