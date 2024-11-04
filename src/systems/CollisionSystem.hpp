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
     * Spatial grid constants and data structures for collision optimization
     * Grid divides world into cells of CELL_SIZE x CELL_SIZE
     * Each cell contains references to entities within that space
     */
    static constexpr float CELL_SIZE = 10.0f;

    struct Cell {
        std::vector<Entity*> entities;
    };

    // Grid storage: Maps cell coordinates to cells containing entities
    static std::unordered_map<int, std::unordered_map<int, Cell>> spatial_grid;

    /**
     * Clears all entities from the spatial grid
     * Called at the start of each collision detection phase
     */
    inline void clear_grid() {
        spatial_grid.clear();
    }

    /**
     * Inserts an entity into the spatial grid based on its position
     * @param entity Pointer to the entity to insert
     * @param pos World position of the entity
     */
    inline void insert_to_grid(Entity* entity, const glm::vec2& pos) {
        int cell_x = static_cast<int>(std::floor(pos.x / CELL_SIZE));
        int cell_y = static_cast<int>(std::floor(pos.y / CELL_SIZE));
        spatial_grid[cell_x][cell_y].entities.push_back(entity);
    }

    /**
     * Retrieves all entities near a given position within a radius
     * Used for broad-phase collision detection
     * @param pos Center position to check around
     * @param radius Radius to check within
     * @return Vector of pointers to nearby entities
     */
    inline std::vector<Entity*> get_nearby_entities(const glm::vec2& pos, float radius) {
        std::vector<Entity*> nearby;
        // Calculate grid cell coordinates for the given position
        int center_x = static_cast<int>(std::floor(pos.x / CELL_SIZE));
        int center_y = static_cast<int>(std::floor(pos.y / CELL_SIZE));

        // Calculate how many cells we need to check based on radius
        int cell_radius = static_cast<int>(std::ceil(radius / CELL_SIZE));

        // Check all cells within the calculated range
        for (int x = center_x - cell_radius; x <= center_x + cell_radius; x++) {
            for (int y = center_y - cell_radius; y <= center_y + cell_radius; y++) {
                auto& cell = spatial_grid[x][y];
                nearby.insert(nearby.end(), cell.entities.begin(), cell.entities.end());
            }
        }
        return nearby;
    }

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
        float distance = glm::length(pos2 - pos1);
        return distance < (c1.radius + c2.radius);
    }

    /**
     * Helper function to check collision between a circle and a line segment
     * Handles both edge and corner collisions for walls
     * @param circle Circle collider to check
     * @param circle_pos Position of circle
     * @param segment Line segment to check against
     * @param offset World space offset for the segment
     * @param out_normal Output parameter for collision normal
     * @param out_penetration Output parameter for penetration depth
     * @return true if collision detected
     */
    inline bool check_circle_segment(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const LineSegment& segment, const glm::vec2& offset,
        glm::vec2& out_normal, float& out_penetration
    ) {
        // Transform segment to world space
        glm::vec2 start = segment.start + offset;
        glm::vec2 end = segment.end + offset;

        // Calculate segment vector
        glm::vec2 segment_vec = end - start;
        float segment_length = glm::length(segment_vec);

        if (segment_length < 0.0001f) return false;  // Degenerate segment

        // Project circle center onto segment line
        glm::vec2 to_circle = circle_pos - start;
        float proj = glm::dot(to_circle, segment_vec) / segment_length;

        // Find closest point on segment to circle
        glm::vec2 closest;
        if (proj <= 0.0f) {
            closest = start;  // Circle is beyond segment start
        } else if (proj >= segment_length) {
            closest = end;    // Circle is beyond segment end
        } else {
            closest = start + (segment_vec * proj / segment_length);  // Circle projects onto segment
        }

        // Check for overlap
        glm::vec2 to_closest = circle_pos - closest;
        float dist = glm::length(to_closest);

        if (dist < circle.radius) {
            // Use segment normal if circle center is exactly on segment
            out_normal = dist > 0.0001f ? to_closest / dist : segment.normal;
            out_penetration = circle.radius - dist;
            return true;
        }

        return false;
    }

    /**
     * Checks collision between a circle and a wall
     * Uses broad-phase AABB check followed by detailed edge checks
     * @param circle Circle collider
     * @param circle_pos Circle position
     * @param wall Wall collider
     * @param wall_pos Wall position
     * @param out_normal Output parameter for collision normal
     * @param out_penetration Output parameter for penetration depth
     * @return true if collision detected
     */
    inline bool check_circle_wall(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const WallCollider& wall, const glm::vec2& wall_pos,
        glm::vec2& out_normal, float& out_penetration
    ) {
        // Broad phase using AABB
        glm::vec2 closest = glm::max(
            wall.aabb.min + wall_pos,
            glm::min(circle_pos, wall.aabb.max + wall_pos)
        );
        if (glm::length(circle_pos - closest) > circle.radius) {
            return false;  // No collision possible
        }

        // Detailed phase: check each edge
        bool collision = false;
        out_penetration = 0.0f;
        out_normal = glm::vec2(0.0f);
        int contact_count = 0;

        // Check collision against each edge of the wall
        for (const auto& edge : wall.edges) {
            glm::vec2 edge_normal;
            float edge_penetration;

            if (check_circle_segment(circle, circle_pos, edge, wall_pos,
                                   edge_normal, edge_penetration)) {
                collision = true;
                out_normal += edge_normal;
                out_penetration = std::max(out_penetration, edge_penetration);
                contact_count++;
            }
        }

        // Average the normals if multiple contacts
        if (collision) {
            out_normal = glm::normalize(out_normal / static_cast<float>(contact_count));
        }

        return collision;
    }

    /**
     * Checks collision between a circle and a convex mesh
     * Uses broad phase with bounding radius followed by detailed edge checks
     * @param circle Circle collider
     * @param circle_pos Circle position
     * @param mesh Mesh collider containing vertices forming a convex hull
     * @param mesh_pos Mesh position
     * @return true if collision detected
     */
    inline bool check_circle_mesh(
        const CircleCollider& circle, const glm::vec2& circle_pos,
        const MeshCollider& mesh, const glm::vec2& mesh_pos
    ) {
        std::cout << "Checking circle-mesh collision with " 
                  << mesh.vertices.size() << " vertices" << std::endl;

        // Broad phase using bounding radius
        if (glm::length(mesh_pos - circle_pos) > (circle.radius + mesh.bound_radius)) {
            return false;
        }

        // Narrow phase: Check each edge of the mesh
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            const glm::vec2& v1 = mesh.vertices[i] + mesh_pos;
            const glm::vec2& v2 = mesh.vertices[(i + 1) % mesh.vertices.size()] + mesh_pos;

            // Create temporary segment for collision check
            LineSegment segment{
                v1 - mesh_pos,
                v2 - mesh_pos,
                glm::normalize(glm::vec2(-(v2.y - v1.y), v2.x - v1.x))  // Perpendicular normal
            };

            glm::vec2 normal;
            float penetration;
            if (check_circle_segment(circle, circle_pos, segment, mesh_pos, normal, penetration)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Main collision detection loop
     * Uses spatial partitioning to optimize collision checks
     */
    inline void check_collisions() {
        Registry& registry = Registry::get_instance();

        // Clear and rebuild spatial grid
        clear_grid();

        // Insert entities into spatial grid
        for (auto& entity : registry.near_players.entities) {
            if (!registry.collision_bounds.has(entity)) continue;
            const auto& motion = registry.motions.get(entity);
            insert_to_grid(&entity, motion.position);
        }

        // Check collisions using spatial grid
        for (auto& entity_i : registry.near_players.entities) {
            if (!registry.collision_bounds.has(entity_i) || registry.death_cooldowns.has(entity_i)) continue;

            const auto& bounds_i = registry.collision_bounds.get(entity_i);
            const auto& motion_i = registry.motions.get(entity_i);

            // Determine radius for broad phase
            float check_radius = 0.0f;
            if (bounds_i.type == ColliderType::Circle) {
                check_radius = bounds_i.circle.radius;
            } else if (bounds_i.type == ColliderType::Wall) {
                // Use half diagonal of AABB for walls
                glm::vec2 half_size = (bounds_i.wall->aabb.max - bounds_i.wall->aabb.min) * 0.5f;
                check_radius = glm::length(half_size);
            } else if (bounds_i.type == ColliderType::Mesh) {
                check_radius = bounds_i.mesh->bound_radius;
            }

            // Get potentially colliding entities from spatial grid
            auto nearby = get_nearby_entities(motion_i.position, check_radius + CELL_SIZE);

            // Check collision with each nearby entity
            for (auto* entity_j_ptr : nearby) {
                Entity& entity_j = *entity_j_ptr;
                if (&entity_i == &entity_j) continue;

                if (!registry.collision_bounds.has(entity_j) || registry.death_cooldowns.has(entity_j)) continue;

                const auto& bounds_j = registry.collision_bounds.get(entity_j);
                const auto& motion_j = registry.motions.get(entity_j);

                // Skip collision check if entities are on the same team
                if (registry.teams.get(entity_i).team_id ==
                    registry.teams.get(entity_j).team_id) {
                    continue;
                }

                bool collision = false;
                glm::vec2 normal;
                float penetration;

                // Handle all possible collider type combinations
                if (bounds_i.type == ColliderType::Circle) {
                    switch (bounds_j.type) {
                        case ColliderType::Circle:
                            collision = check_circle_circle(
                                bounds_i.circle, motion_i.position,
                                bounds_j.circle, motion_j.position
                            );
                            break;

                        case ColliderType::Wall:
                            collision = check_circle_wall(
                                bounds_i.circle, motion_i.position,
                                *bounds_j.wall, motion_j.position,
                                normal, penetration
                            );
                            break;

                        case ColliderType::Mesh:
                            collision = check_circle_mesh(
                                bounds_i.circle, motion_i.position,
                                *bounds_j.mesh, motion_j.position
                            );
                            break;
                        case ColliderType::AABB:
                            // Handle AABB collision or log warning
                            break;
                    }
                }
                // Handle reverse cases (when first entity is not a circle)
                else if (bounds_j.type == ColliderType::Circle) {
                    switch (bounds_i.type) {
                        case ColliderType::Wall:
                            collision = check_circle_wall(
                                bounds_j.circle, motion_j.position,
                                *bounds_i.wall, motion_i.position,
                                normal, penetration
                            );
                            normal = -normal;  // Reverse normal for correct response
                            break;

                        case ColliderType::Mesh:
                            collision = check_circle_mesh(
                                bounds_j.circle, motion_j.position,
                                *bounds_i.mesh, motion_i.position
                            );
                            break;
                        case ColliderType::Circle:
                        case ColliderType::AABB:
                            // Handle or log warning
                            break;
                    }
                }
                else if (bounds_i.type == ColliderType::Mesh) {
                    switch (bounds_j.type) {
                        case ColliderType::Wall:
                            {
                                CircleCollider temp_circle{bounds_i.mesh->bound_radius};
                                collision = check_circle_wall(
                                    temp_circle, motion_i.position,
                                    *bounds_j.wall, motion_j.position,
                                    normal, penetration
                                );
                            }
                            break;
                        case ColliderType::Circle:
                            // Already handled in Circle section
                            break;
                        case ColliderType::Mesh:
                            // Mesh-mesh collision if needed
                            break;
                        case ColliderType::AABB:
                            // Handle AABB collision or log warning
                            break;
                    }
                }
                else if (bounds_i.type == ColliderType::Wall) {
                    switch (bounds_j.type) {
                        case ColliderType::Mesh:
                            {
                                CircleCollider temp_circle{bounds_j.mesh->bound_radius};
                                collision = check_circle_wall(
                                    temp_circle, motion_j.position,
                                    *bounds_i.wall, motion_i.position,
                                    normal, penetration
                                );
                                normal = -normal;  // Reverse normal for correct response
                            }
                            break;
                        case ColliderType::Circle:
                            // Already handled in Circle section
                            break;
                        case ColliderType::Wall:
                            // Wall-wall collision if needed
                            break;
                        case ColliderType::AABB:
                            // Handle AABB collision or log warning
                            break;
                    }
                }

                // Register collision if detected
                if (collision) {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                }
            }
        }
    }

    /**
     * Handles collision between a projectile and a locomotive entity
     * Applies damage and removes projectile
     * @param proj Projectile entity
     * @param loco Locomotive entity (player/enemy)
     */
    inline void proj_loco_collision(Entity& proj, Entity& loco) {
        Registry& registry = Registry::get_instance();
        std::cout << "Projectile collision detected with locomotive entity" << std::endl;

        if (registry.in_dodges.has(loco)) return;

        // Apply damage and poise points to locomotive entity
        auto& loco_stats = registry.locomotion_stats.get(loco);
        auto& projectile = registry.projectiles.get(proj);

        if (std::find(projectile.hit_locos.begin(), projectile.hit_locos.end(), (unsigned int)loco) != projectile.hit_locos.end()) return;
        projectile.hit_locos.push_back(loco);

        loco_stats.health -= projectile.damage;
        loco_stats.poise -= projectile.poise_points;
        if (loco_stats.poise <= 0 && !registry.stagger_cooldowns.has(loco)) {
            registry.stagger_cooldowns.emplace(loco, projectile.stagger_duration);
            loco_stats.poise = loco_stats.max_poise;
        }

        // Remove projectile after hit
        if (projectile.projectile_type == PROJECTILE_TYPE::ARROW) {
            registry.remove_all_components_of(proj);
        }

        // Handle locomotive entity death if health depleted
        if (loco_stats.health <= 0 && !registry.death_cooldowns.has(loco)) {
            // Clean up associated weapon TODO: Drop weapon
            unsigned int weapon_id = registry.attackers.get(loco).weapon_id;
            registry.move_withs.remove(weapon_id);
            registry.rotate_withs.remove(weapon_id);
            // Remove the dead entity
            // registry.remove_all_components_of(loco);
            registry.death_cooldowns.emplace(loco, 5);
        }
    }

    /**
     * Handles collision between two locomotive entities
     * Pushes entities apart based on collision normal
     * @param loco1 First locomotive entity
     * @param loco2 Second locomotive entity
     */
    inline void loco_loco_collision(Entity& loco1, Entity& loco2) {
        Registry& registry = Registry::get_instance();

        Motion& motion1 = registry.motions.get(loco1);
        Motion& motion2 = registry.motions.get(loco2);

        const auto& bounds1 = registry.collision_bounds.get(loco1);
        const auto& bounds2 = registry.collision_bounds.get(loco2);

        // Calculate separation vector
        glm::vec2 delta = motion1.position - motion2.position;
        float distance = glm::length(delta);

        if (distance > 0.0001f) {
            glm::vec2 normal = delta / distance;
            float combined_radius = bounds1.circle.radius + bounds2.circle.radius;
            float overlap = combined_radius - distance;

            if (overlap > 0) {
                const float BUFFER = 0.04f;
                // Push both entities apart equally
                motion1.position += normal * (overlap/2.0f + BUFFER);
                motion2.position -= normal * (overlap/2.0f + BUFFER);

                // Optionally: adjust velocities to prevent sticking
                float vel1_along_normal = glm::dot(motion1.velocity, normal);
                float vel2_along_normal = glm::dot(motion2.velocity, normal);

                if (vel1_along_normal - vel2_along_normal < 0) {
                    // They're moving towards each other, zero out relative velocity
                    glm::vec2 impulse = normal * (vel1_along_normal - vel2_along_normal) * 0.5f;
                    motion1.velocity -= impulse;
                    motion2.velocity += impulse;
                }
            }
        }
    }

    /**
     * Handles collision between a projectile and a fixed entity
     * Simply removes the projectile
     * @param proj Projectile entity
     * @param fixed Fixed entity (wall/obstacle)
     */
    inline void proj_fixed_collision(Entity& proj, Entity& fixed) {
        Registry& registry = Registry::get_instance();
        registry.remove_all_components_of(proj);
    }

    /**
     * Handles collision between a locomotive entity and a fixed entity
     * Implements detailed collision response for walls and other fixed objects
     * @param loco Locomotive entity
     * @param fixed Fixed entity
     */
    inline void loco_fixed_collision(Entity& loco, Entity& fixed) {
        Registry& registry = Registry::get_instance();

        if (!registry.motions.has(loco) || !registry.motions.has(fixed)) {
            return;
        }

        Motion& loco_motion = registry.motions.get(loco);
        const Motion& fixed_motion = registry.motions.get(fixed);
        const auto& loco_bounds = registry.collision_bounds.get(loco);
        const auto& fixed_bounds = registry.collision_bounds.get(fixed);

        const float BUFFER = 0.05f;

        if (fixed_bounds.type == ColliderType::Wall) {
            // Handle wall collision
            glm::vec2 normal;
            float penetration;

            if (check_circle_wall(loco_bounds.circle, loco_motion.position,
                                *fixed_bounds.wall, fixed_motion.position,
                                normal, penetration)) {
                // Apply position correction
                loco_motion.position += normal * (penetration + BUFFER);

                // Adjust velocity for wall sliding
                float vel_along_normal = glm::dot(loco_motion.velocity, normal);
                if (vel_along_normal < 0) {
                    // Remove velocity component in wall normal direction
                    loco_motion.velocity -= normal * vel_along_normal;

                    // Apply wall friction to parallel component
                    glm::vec2 tangent(-normal.y, normal.x);
                    float vel_along_tangent = glm::dot(loco_motion.velocity, tangent);
                    const float WALL_FRICTION = 0.9f;
                    loco_motion.velocity = tangent * vel_along_tangent * WALL_FRICTION;
                }
            }
        } else {
            // Handle other fixed objects (like trees) using circle-circle collision
            glm::vec2 delta = loco_motion.position - fixed_motion.position;
            float dist = glm::length(delta);

            if (dist > 0.0001f) {
                glm::vec2 normal = delta / dist;
                float combined_radius = loco_bounds.circle.radius + fixed_bounds.circle.radius;
                float overlap = combined_radius - dist;

                if (overlap > 0) {
                    // Push locomotive entity away from fixed object
                    loco_motion.position += normal * (overlap + BUFFER);

                    // Adjust velocity to prevent sticking
                    float vel_along_normal = glm::dot(loco_motion.velocity, normal);
                    if (vel_along_normal < 0) {
                        loco_motion.velocity -= normal * vel_along_normal;
                    }
                }
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
     * Priority order: fixed collisions first, then other collisions
     */
    inline void handle_collisions() {
        Registry& registry = Registry::get_instance();

        // Create a separate vector to store collisions
        std::vector<std::pair<Entity, Entity>> collision_pairs;
        for (unsigned int i = 0; i < registry.collisions.entities.size(); i++) {
            Entity& entity1 = registry.collisions.entities[i];
            Entity& entity2 = registry.collisions.get(entity1).other;
            collision_pairs.emplace_back(entity1, entity2);
        }

        // Clear the original collisions vector
        registry.collisions.clear();

        // Process each collision pair
        for (auto& pair : collision_pairs) {
            Entity& entity1 = pair.first;
            Entity& entity2 = pair.second;

            // Check if both entities still exist
            if (!registry.valid(entity1) || !registry.valid(entity2)) {
                continue;
            }

            // Determine collision type and call appropriate handler
            if (registry.projectiles.has(entity1)) {
                if (registry.locomotion_stats.has(entity2)) {
                    proj_loco_collision(entity1, entity2);
                } else {
                    proj_fixed_collision(entity1, entity2);
                }
            } else if (registry.locomotion_stats.has(entity1)) {
                if (registry.projectiles.has(entity2)) {
                    proj_loco_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_loco_collision(entity1, entity2);
                } else {
                    loco_fixed_collision(entity1, entity2);
                }
            } else {
                if (registry.projectiles.has(entity2)) {
                    proj_fixed_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_fixed_collision(entity2, entity1);
                }
            }
        }
    }

} // namespace CollisionSystem