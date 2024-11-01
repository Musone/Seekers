#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include <vector>
#include <glm/geometric.hpp>
#include "AISystem.hpp"
#include "utils/Log.hpp"

namespace CollisionSystem {
    /**
     * Checks collision between two circles using distance-based detection
     * @param c1 First circle collider
     * @param pos1 Position of first circle
     * @param c2 Second circle collider
     * @param pos2 Position of second circle
     * @return true if circles overlap
     */
    inline bool check_circle_circle(
        const CircleCollider& c1, const glm::vec2& pos1,
        const CircleCollider& c2, const glm::vec2& pos2
    ) {
        // Simple circle-circle overlap test: distance < sum of radii
        float distance = glm::length(pos2 - pos1);
        return distance < (c1.radius + c2.radius);
    }

    /**
     * Checks collision between a circle and an Axis-Aligned Bounding Box (AABB)
     * @param circle Circle collider
     * @param circle_pos Circle position
     * @param aabb AABB collider
     * @param aabb_pos AABB position
     * @return true if circle and AABB overlap
     */
    inline bool check_circle_aabb(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const AABBCollider& aabb, const glm::vec2& aabb_pos
    ) {
        // Find closest point on AABB to circle center
        glm::vec2 closest = glm::max(
            aabb.min + aabb_pos,  // Minimum point of AABB in world space
            glm::min(circle_pos, aabb.max + aabb_pos)  // Maximum point of AABB in world space
        );
        
        // Check if closest point is within circle's radius
        float distance = glm::length(closest - circle_pos);
        return distance < circle.radius;
    }

    /**
     * Checks collision between a circle and a convex mesh (weapon/projectile)
     * Uses edge-based detection assuming mesh vertices form a convex hull
     * @param circle Circle collider
     * @param circle_pos Circle position
     * @param mesh Mesh collider containing vertices forming a convex hull
     * @param mesh_pos Mesh position
     * @return true if circle and mesh overlap
     */
    inline bool check_circle_mesh(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const MeshCollider& mesh, const glm::vec2& mesh_pos
    ) {
        // Broad phase: Quick check using bounding radius
        // If objects are too far apart, no need for detailed check
        if (glm::length(mesh_pos - circle_pos) > (circle.radius + mesh.bound_radius)) {
            return false;
        }

        // Narrow phase: Check each edge of the convex hull
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            // Get edge vertices in world space
            const glm::vec2& v1 = mesh.vertices[i] + mesh_pos;
            const glm::vec2& v2 = mesh.vertices[(i + 1) % mesh.vertices.size()] + mesh_pos;
            
            // Calculate edge vector and normalize it
            glm::vec2 line = v2 - v1;
            float len = glm::length(line);
            glm::vec2 dir = line / len;
            
            // Project circle center onto edge line
            float t = glm::dot(circle_pos - v1, dir);
            // Clamp projection to edge endpoints
            t = glm::clamp(t, 0.0f, len);
            
            // Find closest point on edge to circle center
            glm::vec2 closest = v1 + dir * t;
            // Check if closest point is within circle's radius
            if (glm::length(circle_pos - closest) < circle.radius) {
                return true;
            }
        }
        return false;
    }

    /**
     * Calculates collision normal between two colliders
     * Normal points from second collider to first collider
     * @param bounds1 First collider bounds
     * @param pos1 First collider position
     * @param bounds2 Second collider bounds
     * @param pos2 Second collider position
     * @return Normalized collision normal vector
     */
    inline glm::vec2 get_collision_normal(
        const CollisionBounds& bounds1, const glm::vec2& pos1,
        const CollisionBounds& bounds2, const glm::vec2& pos2
    ) {
        if (bounds1.type == ColliderType::Circle && bounds2.type == ColliderType::Circle) {
            // For circle-circle, normal is along centers
            return glm::normalize(pos1 - pos2);
        }
        else if (bounds2.type == ColliderType::AABB) {
            // For circle-AABB, normal is from closest point on AABB to circle center
            glm::vec2 closest = glm::max(
                bounds2.aabb.min + pos2,
                glm::min(pos1, bounds2.aabb.max + pos2)
            );
            return glm::normalize(pos1 - closest);
        }
        else if (bounds2.type == ColliderType::Mesh) {
            // For circle-mesh, find closest edge and use its normal
            float min_dist = std::numeric_limits<float>::max();
            glm::vec2 closest_normal(0.0f);
            
            // Check each edge of the mesh
            for (size_t i = 0; i < bounds2.mesh->vertices.size(); i++) {
                const glm::vec2& v1 = bounds2.mesh->vertices[i] + pos2;
                const glm::vec2& v2 = bounds2.mesh->vertices[(i + 1) % bounds2.mesh->vertices.size()] + pos2;
                
                glm::vec2 line = v2 - v1;
                float len = glm::length(line);
                glm::vec2 dir = line / len;
                
                // Project point onto edge
                float t = glm::dot(pos1 - v1, dir);
                t = glm::clamp(t, 0.0f, len);
                
                glm::vec2 closest = v1 + dir * t;
                float dist = glm::length(pos1 - closest);
                
                // Keep track of closest edge
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_normal = glm::normalize(pos1 - closest);
                }
            }
            return closest_normal;
        }
        return glm::vec2(0.0f);
    }

    /**
     * Calculates penetration depth between two colliders
     * @param bounds1 First collider bounds
     * @param pos1 First collider position
     * @param bounds2 Second collider bounds
     * @param pos2 Second collider position
     * @return Depth of penetration (positive indicates overlap)
     */
    inline float get_penetration_depth(
        const CollisionBounds& bounds1, const glm::vec2& pos1,
        const CollisionBounds& bounds2, const glm::vec2& pos2
    ) {
        if (bounds1.type == ColliderType::Circle && bounds2.type == ColliderType::Circle) {
            // For circle-circle, penetration is difference between sum of radii and actual distance
            float distance = glm::length(pos2 - pos1);
            return bounds1.circle.radius + bounds2.circle.radius - distance;
        }
        else if (bounds2.type == ColliderType::AABB) {
            // For circle-AABB, penetration is difference between circle radius and distance to closest point
            glm::vec2 closest = glm::max(
                bounds2.aabb.min + pos2,
                glm::min(pos1, bounds2.aabb.max + pos2)
            );
            return bounds1.circle.radius - glm::length(closest - pos1);
        }
        else if (bounds2.type == ColliderType::Mesh) {
            // For circle-mesh, find minimum penetration against all edges
            float min_dist = std::numeric_limits<float>::max();
            
            for (size_t i = 0; i < bounds2.mesh->vertices.size(); i++) {
                const glm::vec2& v1 = bounds2.mesh->vertices[i] + pos2;
                const glm::vec2& v2 = bounds2.mesh->vertices[(i + 1) % bounds2.mesh->vertices.size()] + pos2;
                
                glm::vec2 line = v2 - v1;
                float len = glm::length(line);
                glm::vec2 dir = line / len;
                
                float t = glm::dot(pos1 - v1, dir);
                t = glm::clamp(t, 0.0f, len);
                
                glm::vec2 closest = v1 + dir * t;
                min_dist = std::min(min_dist, glm::length(pos1 - closest));
            }
            return bounds1.circle.radius - min_dist;
        }
        return 0.0f;
    }

    /**
     * Main collision detection loop
     * Checks for collisions between all relevant entity pairs
     */
    inline void check_collisions() {
        Registry& registry = Registry::get_instance();

        // Iterate through all entities that are near players
        for (unsigned int i = 0; i < registry.near_players.entities.size(); i++) {
            Entity& entity_i = registry.near_players.entities[i];
            if (!registry.collision_bounds.has(entity_i)) continue;

            const auto& bounds_i = registry.collision_bounds.get(entity_i);
            const auto& motion_i = registry.motions.get(entity_i);

            // Compare against all other entities
            for(unsigned int j = i+1; j < registry.near_players.entities.size(); j++) {
                Entity& entity_j = registry.near_players.entities[j];
                if (!registry.collision_bounds.has(entity_j)) continue;

                const auto& bounds_j = registry.collision_bounds.get(entity_j);
                const auto& motion_j = registry.motions.get(entity_j);

                // Skip collision check if entities are on the same team
                if (registry.teams.get(entity_i).team_id == registry.teams.get(entity_j).team_id) {
                    continue;
                }

                bool collision = false;

                // Handle all possible collider type combinations
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

                // If collision detected, register it for handling
                if (collision) {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                }
            }
        }
    }

    /**
     * Handles collision between a projectile and a locomotive entity (player/enemy)
     * @param proj Projectile entity
     * @param loco Locomotive entity
     */
    inline void proj_loco_collision(Entity& proj, Entity& loco) {
        Registry& registry = Registry::get_instance();

        // Apply damage to locomotive entity
        registry.locomotion_stats.get(loco).health -= registry.projectile_stats.get(proj).damage;
        // Remove projectile after hit
        registry.remove_all_components_of(proj);

        // Handle locomotive entity death
        if (registry.locomotion_stats.get(loco).health <= 0) {
            // Clean up associated weapon
            unsigned int weapon_id = registry.attackers.get(loco).weapon_id;
            registry.move_withs.remove(weapon_id);
            registry.rotate_withs.remove(weapon_id);
            // Remove the dead entity
            registry.remove_all_components_of(loco);
        }
    }

    /**
     * Handles collision between two locomotive entities (player/enemy)
     * @param loco1 First locomotive entity
     * @param loco2 Second locomotive entity
     */
    inline void loco_loco_collision(Entity& loco1, Entity& loco2) {
        Registry& registry = Registry::get_instance();

        Motion& motion1 = registry.motions.get(loco1);
        Motion& motion2 = registry.motions.get(loco2);
        
        const auto& bounds1 = registry.collision_bounds.get(loco1);
        const auto& bounds2 = registry.collision_bounds.get(loco2);

        // Calculate collision response
        glm::vec2 normal = get_collision_normal(bounds1, motion1.position, bounds2, motion2.position);
        float penetration = get_penetration_depth(bounds1, motion1.position, bounds2, motion2.position);

        // Push entities apart with a small buffer to prevent sticking
        const float BUFFER = 0.04f;
        motion1.position += normal * (penetration/2.0f + BUFFER);
        motion2.position -= normal * (penetration/2.0f + BUFFER);
    }

    /**
     * Handles collision between a projectile and a fixed entity (wall/obstacle)
     * @param proj Projectile entity
     * @param fixed Fixed entity
     */
    inline void proj_fixed_collision(Entity& proj, Entity& fixed) {
        Registry& registry = Registry::get_instance();
        // Simply remove projectile on impact with fixed object
        registry.remove_all_components_of(proj);
    }

    /**
     * Handles collision between a locomotive entity and a fixed entity
     * @param loco Locomotive entity
     * @param fixed Fixed entity
     */
    /**
     * Handles collision between a locomotive entity and a fixed entity
     * @param loco Locomotive entity
     * @param fixed Fixed entity
     */
    inline void loco_fixed_collision(Entity& loco, Entity& fixed) {
        Registry& registry = Registry::get_instance();
        
        if (!registry.motions.has(loco) || !registry.motions.has(fixed)) {
            return;
        }
        
        Motion& loco_motion = registry.motions.get(loco);
        Motion& fixed_motion = registry.motions.get(fixed);
        
        const auto& loco_bounds = registry.collision_bounds.get(loco);
        const auto& fixed_bounds = registry.collision_bounds.get(fixed);

        const float BUFFER = 0.05f;
        
        if (fixed_bounds.type == ColliderType::AABB) {
            // Store original position and velocity
            glm::vec2 original_pos = loco_motion.position;
            glm::vec2 velocity = loco_motion.velocity;
            
            // Expand AABB by circle radius (Minkowski sum)
            glm::vec2 expanded_min = fixed_bounds.aabb.min + fixed_motion.position - glm::vec2(loco_bounds.circle.radius);
            glm::vec2 expanded_max = fixed_bounds.aabb.max + fixed_motion.position + glm::vec2(loco_bounds.circle.radius);
            
            // Check if already inside expanded AABB
            bool inside_x = original_pos.x >= expanded_min.x && original_pos.x <= expanded_max.x;
            bool inside_y = original_pos.y >= expanded_min.y && original_pos.y <= expanded_max.y;
            
            if (inside_x && inside_y) {
                // Find closest edge to push out
                float dist_left = original_pos.x - expanded_min.x;
                float dist_right = expanded_max.x - original_pos.x;
                float dist_top = original_pos.y - expanded_min.y;
                float dist_bottom = expanded_max.y - original_pos.y;
                
                // Find minimum penetration
                float min_dist = std::min({dist_left, dist_right, dist_top, dist_bottom});
                
                // Push out in direction of minimum penetration
                if (min_dist == dist_left) loco_motion.position.x = expanded_min.x - BUFFER;
                else if (min_dist == dist_right) loco_motion.position.x = expanded_max.x + BUFFER;
                else if (min_dist == dist_top) loco_motion.position.y = expanded_min.y - BUFFER;
                else loco_motion.position.y = expanded_max.y + BUFFER;
            } else {
                // Swept collision detection
                float entry_time = 0.0f;
                float exit_time = 1.0f;
                glm::vec2 normal(0.0f);
                
                if (velocity.x != 0.0f) {
                    float inv_entry_x = (expanded_min.x - original_pos.x) / velocity.x;
                    float inv_exit_x = (expanded_max.x - original_pos.x) / velocity.x;
                    
                    entry_time = std::max(entry_time, std::min(inv_entry_x, inv_exit_x));
                    exit_time = std::min(exit_time, std::max(inv_entry_x, inv_exit_x));
                    
                    if (inv_entry_x < inv_exit_x) normal.x = -1.0f;
                    else normal.x = 1.0f;
                }
                
                if (velocity.y != 0.0f) {
                    float inv_entry_y = (expanded_min.y - original_pos.y) / velocity.y;
                    float inv_exit_y = (expanded_max.y - original_pos.y) / velocity.y;
                    
                    entry_time = std::max(entry_time, std::min(inv_entry_y, inv_exit_y));
                    exit_time = std::min(exit_time, std::max(inv_entry_y, inv_exit_y));
                    
                    if (inv_entry_y < inv_exit_y) normal.y = -1.0f;
                    else normal.y = 1.0f;
                }
                
                // Check if collision occurs
                if (entry_time <= exit_time && exit_time > 0.0f && entry_time < 1.0f) {
                    // Move to collision point
                    loco_motion.position = original_pos + velocity * std::max(0.0f, entry_time - BUFFER);
                    
                    // Reflect velocity along collision normal
                    if (abs(normal.x) > 0.0f) {
                        loco_motion.velocity.x = 0.0f;
                    }
                    if (abs(normal.y) > 0.0f) {
                        loco_motion.velocity.y = 0.0f;
                    }
                }
            }
            
            // Additional corner handling
            const auto& aabb = fixed_bounds.aabb;
            glm::vec2 corners[4] = {
                fixed_motion.position + aabb.min,                          // Bottom-left
                fixed_motion.position + glm::vec2(aabb.max.x, aabb.min.y), // Bottom-right
                fixed_motion.position + aabb.max,                          // Top-right
                fixed_motion.position + glm::vec2(aabb.min.x, aabb.max.y)  // Top-left
            };

            // Check all corners
            for (const auto& corner : corners) {
                glm::vec2 to_corner = loco_motion.position - corner;
                float corner_dist = glm::length(to_corner);
                
                if (corner_dist < loco_bounds.circle.radius) {
                    // We're too close to a corner
                    if (corner_dist > 0.0001f) {
                        glm::vec2 corner_normal = to_corner / corner_dist;
                        loco_motion.position = corner + corner_normal * (loco_bounds.circle.radius + BUFFER);
                        
                        // Adjust velocity to prevent corner sticking
                        float vel_along_normal = glm::dot(loco_motion.velocity, corner_normal);
                        if (vel_along_normal < 0) {
                            loco_motion.velocity -= corner_normal * vel_along_normal;
                        }
                    } else {
                        // If exactly on corner, push out diagonally
                        loco_motion.position += glm::vec2(1.0f, 1.0f) * (loco_bounds.circle.radius + BUFFER);
                    }
                }
            }
        } else {
            // Circle-Circle collision handling for fixed entities (trees, etc.)
            // Calculate centers and combined radius
            glm::vec2 diff = loco_motion.position - fixed_motion.position;
            float dist = glm::length(diff);
            
            if (dist < 0.0001f) {
                // Prevent division by zero
                diff = glm::vec2(1.0f, 0.0f);
                dist = 1.0f;
            }
            
            float combined_radius = loco_bounds.circle.radius + fixed_bounds.circle.radius;
            
            if (dist < combined_radius) {
                // Calculate penetration and normal
                float penetration = combined_radius - dist;
                glm::vec2 normal = diff / dist;
                
                // Push locomotive entity out
                loco_motion.position += normal * (penetration + BUFFER);
                
                // Adjust velocity to prevent sticking
                float vel_along_normal = glm::dot(loco_motion.velocity, normal);
                if (vel_along_normal < 0) {
                    loco_motion.velocity -= normal * vel_along_normal;
                }
            }
        }
        
        // Final position validation
        if (fixed_bounds.type == ColliderType::AABB) {
            glm::vec2 closest = glm::max(
                fixed_bounds.aabb.min + fixed_motion.position,
                glm::min(loco_motion.position, fixed_bounds.aabb.max + fixed_motion.position)
            );
            
            float dist = glm::length(loco_motion.position - closest);
            if (dist < loco_bounds.circle.radius) {
                // If still penetrating, force position to safe distance
                glm::vec2 normal = (loco_motion.position - closest) / dist;
                loco_motion.position = closest + normal * (loco_bounds.circle.radius + BUFFER);
            }
        }
        
        // Update AI and dodge status
        if (registry.ais.has(loco)) {
            AISystem::update_patrol_target_position(registry.ais.get(loco));
        }
        if (registry.in_dodges.has(loco)) {
            registry.in_dodges.remove(loco);
        }
    }

    /**
     * Main collision handling function
     * Processes all detected collisions and applies appropriate responses
     */
    inline void handle_collisions() {
        Registry& registry = Registry::get_instance();
        
        for (unsigned int i = 0; i < registry.collisions.entities.size(); i++) {
            Entity& entity1 = registry.collisions.entities[i];
            Entity& entity2 = registry.collisions.get(entity1).other;

            // Skip if either entity was removed in previous collision handling
            if (!registry.teams.has(entity2) || !registry.teams.has(entity1)) {
                continue;
            }

            // Determine collision type and call appropriate handler
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
        // Clear processed collisions
        registry.collisions.clear();
    }
};