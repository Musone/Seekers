#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include <vector>
#include <glm/geometric.hpp>

#include "AISystem.hpp"
#include "utils/Log.hpp"

namespace CollisionSystem {
    // Collision detection helpers
    inline bool check_circle_circle(
        const CircleCollider& c1, const glm::vec2& pos1,
        const CircleCollider& c2, const glm::vec2& pos2
    ) {
        // Simple distance check between centers - if less than combined radii, circles overlap.
        float distance = glm::length(pos2 - pos1);
        return distance < (c1.radius + c2.radius);
    }

    inline bool check_circle_aabb(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const AABBCollider& aabb, const glm::vec2& aabb_pos
    ) {
        // Find closest point on AABB to circle by:
        // 1. Clamping circle's position to box bounds
        // 2. If distance from circle center to closest point < radius, they collide
        glm::vec2 closest = glm::max(
            aabb.min + aabb_pos,
            glm::min(circle_pos, aabb.max + aabb_pos)
        );
        // Calculate distance between circle center and closest point
        float distance = glm::length(closest - circle_pos);
        // If distance is less than circle radius, they collide
        return distance < circle.radius;
    }

    inline bool check_circle_mesh(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const MeshCollider& mesh, const glm::vec2& mesh_pos
    ) {
        // First check broad phase using bound radius
        if (glm::length(mesh_pos - circle_pos) > (circle.radius + mesh.bound_radius)) {
            return false;
        }

        // Then check each edge of the mesh
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            const glm::vec2& v1 = mesh.vertices[i] + mesh_pos;
            const glm::vec2& v2 = mesh.vertices[(i + 1) % mesh.vertices.size()] + mesh_pos;
            
            // Check distance from circle to line segment
            glm::vec2 line = v2 - v1;
            float len = glm::length(line);
            glm::vec2 dir = line / len;
            
            float t = glm::dot(circle_pos - v1, dir);
            t = glm::clamp(t, 0.0f, len);
            
            glm::vec2 closest = v1 + dir * t;
            if (glm::length(circle_pos - closest) < circle.radius) {
                return true;
            }
        }
        return false;
    }

    inline void check_collisions() {
        Registry& registry = Registry::get_instance();

        for (unsigned int i = 0; i < registry.near_players.entities.size(); i++) {
            Entity& entity_i = registry.near_players.entities[i];
            if (!registry.collision_bounds.has(entity_i)) continue;

            const auto& bounds_i = registry.collision_bounds.get(entity_i);
            const auto& motion_i = registry.motions.get(entity_i);

            for(unsigned int j = i+1; j < registry.near_players.entities.size(); j++) {
                Entity& entity_j = registry.near_players.entities[j];
                if (!registry.collision_bounds.has(entity_j)) continue;

                const auto& bounds_j = registry.collision_bounds.get(entity_j);
                const auto& motion_j = registry.motions.get(entity_j);

                if (registry.teams.get(entity_i).team_id == registry.teams.get(entity_j).team_id) {
                    continue;
                }

                bool collision = false;

                // Handle all collision type combinations
                if (bounds_i.type == ColliderType::Circle && bounds_j.type == ColliderType::Circle) {
                    collision = check_circle_circle(
                        bounds_i.circle, motion_i.position,
                        bounds_j.circle, motion_j.position
                    );
                }
                else if (bounds_i.type == ColliderType::Circle && bounds_j.type == ColliderType::AABB) {
                    collision = check_circle_aabb(
                        bounds_i.circle, motion_i.position,
                        bounds_j.aabb, motion_j.position
                    );
                }
                else if (bounds_i.type == ColliderType::AABB && bounds_j.type == ColliderType::Circle) {
                    collision = check_circle_aabb(
                        bounds_j.circle, motion_j.position,
                        bounds_i.aabb, motion_i.position
                    );
                }
                else if (bounds_i.type == ColliderType::Circle && bounds_j.type == ColliderType::Mesh) {
                    collision = check_circle_mesh(
                        bounds_i.circle, motion_i.position,
                        *bounds_j.mesh, motion_j.position
                    );
                }
                else if (bounds_i.type == ColliderType::Mesh && bounds_j.type == ColliderType::Circle) {
                    collision = check_circle_mesh(
                        bounds_j.circle, motion_j.position,
                        *bounds_i.mesh, motion_i.position
                    );
                }

                if (collision) {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                }
            }
        }
    }

    inline void proj_loco_collision(Entity& proj, Entity& loco) {
        Registry& registry = Registry::get_instance();

        registry.locomotion_stats.get(loco).health -= registry.projectile_stats.get(proj).damage;
        registry.remove_all_components_of(proj);
        //Log::log_info("Projectile hit target.", __FILE__, __LINE__);
        if (registry.locomotion_stats.get(loco).health <= 0) {
            unsigned int weapon_id = registry.attackers.get(loco).weapon_id;
            registry.move_withs.remove(weapon_id);
            registry.rotate_withs.remove(weapon_id);
            registry.remove_all_components_of(loco);
        }
    }

    inline void loco_loco_collision(Entity& loco1, Entity& loco2) {
        Registry& registry = Registry::get_instance();

        // Log::log_info("Locomotion entities collided.", __FILE__, __LINE__);
        Motion& motion1 = registry.motions.get(loco1);
        Motion& motion2 = registry.motions.get(loco2);
        glm::vec2 delta = motion1.position - motion2.position;
        float overlap = registry.collision_bounds.get(loco1).circle.radius + registry.collision_bounds.get(loco2).circle.radius - glm::length(delta);
        glm::vec2 direction = glm::normalize(delta);
        motion1.position += direction * (overlap + 0.04f);
        motion2.position -= direction * (overlap + 0.04f);
    }

    inline void proj_fixed_collision(Entity& proj, Entity& fixed) {
        Registry& registry = Registry::get_instance();

        registry.remove_all_components_of(proj);
        // Log::log_info("Projectile hit a fixed entity.", __FILE__, __LINE__);
    }

    inline void loco_fixed_collision(Entity& loco, Entity& fixed) {
        Registry& registry = Registry::get_instance();

        // Log::log_info("Locomotion entity collided with fixed entity.", __FILE__, __LINE__);
        if (!registry.motions.has(loco) || !registry.motions.has(fixed)) {return;}
        Motion& loco_motion = registry.motions.get(loco);
        Motion& fixed_motion = registry.motions.get(fixed);
        auto pushback_dir = Common::normalize(loco_motion.velocity) * -1.0f;
        glm::vec2 delta = loco_motion.position - fixed_motion.position;
        float overlap = registry.collision_bounds.get(loco).circle.radius + registry.collision_bounds.get(fixed).circle.radius - glm::length(delta);
        glm::vec2 direction = glm::normalize(delta);
        loco_motion.position += pushback_dir * (overlap + 0.04f);
        delta = loco_motion.position - fixed_motion.position;
        overlap = registry.collision_bounds.get(loco).circle.radius + registry.collision_bounds.get(fixed).circle.radius - glm::length(delta);
        if (overlap > 0.0f) {
            loco_motion.position += direction * (overlap + 0.04f);
        }
        if (registry.ais.has(loco)) {
            AISystem::update_patrol_target_position(registry.ais.get(loco));
        }
        if (registry.in_dodges.has(loco)) {registry.in_dodges.remove(loco);}
    }

    inline void handle_collisions()
    {
        Registry& registry = Registry::get_instance();
        for (unsigned int i = 0; i < registry.collisions.entities.size(); i++) {
            Entity& entity1 = registry.collisions.entities[i];
            Entity& entity2 = registry.collisions.get(entity1).other;

            // check if one of the entities were removed in previous collision resolve
            if (!registry.teams.has(entity2) || !registry.teams.has(entity1)) {continue;}

            if (registry.projectile_stats.has(entity1)) {
                if (registry.locomotion_stats.has(entity2)) {
                    proj_loco_collision(entity1, entity2);
                } else {
                    proj_fixed_collision(entity1, entity2);
                }
            } else if (registry.locomotion_stats.has(entity1)) {
                if (registry.projectile_stats.has(entity2)) {
                    proj_loco_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_loco_collision(entity1, entity2);
                } else {
                    loco_fixed_collision(entity1, entity2);
                }
            } else {
                if (registry.projectile_stats.has(entity2)) {
                    proj_fixed_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_fixed_collision(entity2, entity1);
                }
            }
        }
        registry.collisions.clear();
    }
};
