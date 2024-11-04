#pragma once

#include <ecs/Registry.hpp>
#include <ecs/Entity.hpp>
#include <components/Components.hpp>
#include <utils/Common.hpp>

#include <glm/glm.hpp>

namespace EntityFactory {
    inline Entity create_player(glm::vec2 position) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(3.0f, 3.0f);  // Player size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 200.0f;
        locomotion.max_health = 200.0f;
        locomotion.movement_speed = 15.0f;
        locomotion.max_energy = 100.0f;
        locomotion.energy = locomotion.max_energy;
        locomotion.max_poise = 30.0f;
        locomotion.poise = locomotion.max_poise;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FRIENDLY;

        registry.attackers.emplace(entity);

        // Use circle collider for player
        auto& bounds = registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_weapon(glm::vec2 position, float damage, float attack_cooldown = 0.5f, WEAPON_TYPE weapon_type = WEAPON_TYPE::SWORD) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.5f, 1.5f);

        auto& weapon = registry.weapons.emplace(entity);
        weapon.type = weapon_type;
        weapon.damage = damage;
        if (weapon_type == WEAPON_TYPE::BOW) {
            weapon.range = 30.0f;
        } else {
            weapon.range = 5.0f;
        }
        weapon.proj_speed = 80.0f;
        weapon.attack_cooldown = attack_cooldown;
        weapon.stagger_duration = 0.5f;
        weapon.poise_points = 10.0f;
        weapon.attack_energy_cost = 10.0f;
        if (weapon_type == WEAPON_TYPE::BOW) {
            weapon.projectile_type = PROJECTILE_TYPE::ARROW;
        } else {
            weapon.projectile_type = PROJECTILE_TYPE::MELEE;
        }
        weapon.enchantment = ENCHANTMENT::NONE;

        return entity;
    }

    inline Entity create_enemy(glm::vec2 position, ENEMY_TYPE enemy_type = ENEMY_TYPE::WARRIOR) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(3.0f, 3.0f);  // Enemy size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 50.0f;
        locomotion.max_health = 50.0f;
        locomotion.max_energy = 100.0f;
        locomotion.energy = locomotion.max_energy;
        locomotion.max_poise = 10.0f;
        locomotion.poise = locomotion.max_poise;
        locomotion.movement_speed = 10.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        auto& attacker = registry.attackers.emplace(entity);

        AIComponent& ai = registry.ais.emplace(entity);
        ai.current_state = AI_STATE::PATROL;
        ai.patrol_points.push_back(position);
        ai.patrol_points.push_back(position + glm::vec2(10.0f, 0.0f));
        ai.target_position = position;

        auto& enemy = registry.enemies.emplace(entity);
        enemy.type = enemy_type;

        Entity enemy_weapon;
        if (enemy_type == ENEMY_TYPE::ZOMBIE) {
            enemy_weapon = EntityFactory::create_weapon(position, 5.0f, 0.5f, WEAPON_TYPE::PUNCH);
        } else if (enemy_type == ENEMY_TYPE::ARCHER) {
            enemy_weapon = EntityFactory::create_weapon(position, 5.0f, 0.5f, WEAPON_TYPE::BOW);
        } else {
            enemy_weapon = EntityFactory::create_weapon(position, 5.0f, 0.5f, WEAPON_TYPE::SWORD);
        }
        attacker.weapon_id = enemy_weapon;

        // Use circle collider for enemy
        auto& bounds = registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_projectile(Motion& attacker_motion, Attacker& attacker, Weapon& weapon, TEAM_ID team_id) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = attacker_motion.position;
        // motion.angle = atan2(attacker.aim.y, attacker.aim.x);
        motion.angle = attacker_motion.angle; // Changed this so that I can render projectiles properly in 3d-mode.
        //motion.velocity = attacker.aim * weapon.proj_speed + attacker_motion.velocity;
        motion.velocity = attacker.aim * weapon.proj_speed;
        motion.scale = glm::vec2(1.0f, 1.0f);  // Projectile size

        auto& projectile = registry.projectiles.emplace(entity);
        projectile.damage = weapon.damage;
        projectile.range_remaining = weapon.range;
        projectile.stagger_duration = weapon.stagger_duration;
        projectile.poise_points = weapon.poise_points;
        projectile.enchantment = ENCHANTMENT::NONE;
        projectile.projectile_type = weapon.projectile_type;

        auto& team = registry.teams.emplace(entity);
        team.team_id = team_id;

        // Get the appropriate model from registry
        StaticModel* projectile_model = nullptr;
        if (!registry.projectile_models.entities.empty()) {
            const auto& models = registry.projectile_models.components[0];
            if (weapon.projectile_type == PROJECTILE_TYPE::ARROW) {
                projectile_model = models.arrow_model;
            } else if (weapon.projectile_type == PROJECTILE_TYPE::MELEE) {
                projectile_model = models.melee_model;
            }
        }

        // Create collision mesh from model if available
        if (projectile_model && projectile_model->mesh_list.size() > 0) {
            // Find mesh with most triangles
            const Mesh* best_mesh = nullptr;
            size_t max_triangles = 0;
            
            std::cout << "Checking " << projectile_model->mesh_list.size() 
                      << " meshes for projectile type: " 
                      << (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") << std::endl;
            
            for (size_t i = 0; i < projectile_model->mesh_list.size(); i++) {
                const Mesh* current_mesh = projectile_model->mesh_list[i].get();
                size_t triangle_count = current_mesh->triangles.size();
                std::cout << "Mesh " << i << " has " << triangle_count << " triangles" << std::endl;
                
                if (triangle_count > max_triangles) {
                    max_triangles = triangle_count;
                    best_mesh = current_mesh;
                }
            }
            
            if (!best_mesh) {
                std::cout << "No valid meshes found with triangles" << std::endl;
                auto& bounds = registry.collision_bounds.emplace(entity,
                    CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
                return entity;
            }

            const Mesh* mesh = best_mesh;
            std::cout << "Using mesh with " << max_triangles << " triangles" << std::endl;
            
            // Get model scale from the model itself
            glm::vec3 model_scale = projectile_model->get_scale();
            
            // Create transformation matrices
            glm::mat4 pre_transform = projectile_model->get_pre_transform();
            glm::mat4 model_matrix = Transform::create_model_matrix(
                glm::vec3(0, 0, 0),  // We'll add position later
                glm::vec3(0, 0, motion.angle),  // Only rotate around Z
                glm::vec3(model_scale)  // Use model scale instead of motion scale
            );
            
            // Transform and collect vertices
            std::vector<glm::vec2> vertices_2d;
            auto add_unique_vertex = [&vertices_2d](const glm::vec2& v) {
                const float EPSILON = 0.0001f;
                for (const auto& existing : vertices_2d) {
                    if (glm::length(existing - v) < EPSILON) return; // Already exists
                }
                vertices_2d.push_back(v);
            };

            for (const Triangle& tri : mesh->triangles) {
                // Transform vertices through pre-transform and model matrix
                glm::vec4 v0 = model_matrix * pre_transform * glm::vec4(tri.v0, 1.0f);
                glm::vec4 v1 = model_matrix * pre_transform * glm::vec4(tri.v1, 1.0f);
                glm::vec4 v2 = model_matrix * pre_transform * glm::vec4(tri.v2, 1.0f);
                
                // Project to 2D and add unique vertices
                add_unique_vertex(glm::vec2(v0.x, v0.y));
                add_unique_vertex(glm::vec2(v1.x, v1.y));
                add_unique_vertex(glm::vec2(v2.x, v2.y));
            }
            
            if (vertices_2d.empty()) {
                std::cout << "No vertices found in mesh, using circle collider" << std::endl;
                auto& bounds = registry.collision_bounds.emplace(entity,
                    CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
            } else {
                // Compute centroid
                glm::vec2 centroid(0.0f);
                for (const auto& v : vertices_2d) {
                    centroid += v;
                }
                centroid /= static_cast<float>(vertices_2d.size());
                
                // Sort vertices by angle around centroid
                std::sort(vertices_2d.begin(), vertices_2d.end(),
                    [centroid](const glm::vec2& a, const glm::vec2& b) {
                        return atan2(a.y - centroid.y, a.x - centroid.x) <
                               atan2(b.y - centroid.y, b.x - centroid.x);
                    });
                
                // Calculate bound radius
                float bound_radius = 0.0f;
                for (const auto& vertex : vertices_2d) {
                    bound_radius = std::max(bound_radius, glm::length(vertex - centroid));
                }
                
                std::cout << "Creating mesh collider with " << vertices_2d.size() 
                          << " vertices and radius " << bound_radius << std::endl;
                
                auto& bounds = registry.collision_bounds.emplace(entity,
                    CollisionBounds::create_mesh(vertices_2d, bound_radius));
            }
        } else {
            std::cout << "No valid mesh found for projectile, using circle collider" << std::endl;
            auto& bounds = registry.collision_bounds.emplace(entity,
                CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
        }

        return entity;
    }

    inline Entity create_wall(glm::vec2 position, float angle, glm::vec2 scale = glm::vec2(2.0f, 2.0f)) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.angle = angle;
        motion.scale = scale;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& wall = registry.walls.emplace(entity);
        wall.type = WALL_TYPE::BRICK;

        // // Use AABB collider for wall
        // auto& bounds = registry.collision_bounds.emplace(entity,
        //     CollisionBounds::create_aabb(motion.scale));

        // Use wall collider instead of AABB
        auto& bounds = registry.collision_bounds.emplace(entity,
            CollisionBounds::create_wall(motion.scale, motion.angle));

        return entity;
    }

    inline Entity create_no_collision_wall(glm::vec2 position, float angle, glm::vec2 scale = glm::vec2(2.0f, 2.0f)) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.angle = angle;
        motion.scale = scale;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& wall = registry.walls.emplace(entity);
        wall.type = WALL_TYPE::BRICK;

        return entity;
    }

    inline Entity create_tree(glm::vec2 position) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(4.0f, 4.0f);

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& tree = registry.static_objects.emplace(entity);
        tree.type = STATIC_OBJECT_TYPE::TREE;

        // Use circle collider for tree
        auto& bounds = registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        registry.rotate_withs.emplace(entity, registry.player);

        return entity;
    }
};
