#pragma once

#include <ecs/Registry.hpp>
#include <ecs/Entity.hpp>
#include <components/Components.hpp>
#include <utils/Common.hpp>

#include <glm/glm.hpp>

namespace EntityFactory {
    inline Entity create_player(Registry& registry, glm::vec2 position) {
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
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        registry.player = entity;

        return entity;
    }

    inline Entity create_weapon(Registry& registry, glm::vec2 position, float damage, float attack_cooldown = 0.5f, WEAPON_TYPE weapon_type = WEAPON_TYPE::SWORD) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.5f, 1.5f);

        auto& weapon = registry.weapons.emplace(entity);
        weapon.type = weapon_type;
        weapon.damage = damage;
        if (weapon_type == WEAPON_TYPE::BOW) {
            weapon.range = 30.0f;
        } else if (weapon_type == WEAPON_TYPE::SWORD) {
            weapon.range = 10.0f;
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

    inline Entity create_enemy(Registry& registry, glm::vec2 position, ENEMY_TYPE enemy_type = ENEMY_TYPE::WARRIOR) {
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
            enemy_weapon = EntityFactory::create_weapon(registry, position, 5.0f, 0.5f, WEAPON_TYPE::PUNCH);
        } else if (enemy_type == ENEMY_TYPE::ARCHER) {
            enemy_weapon = EntityFactory::create_weapon(registry, position, 5.0f, 0.5f, WEAPON_TYPE::BOW);
        } else {
            enemy_weapon = EntityFactory::create_weapon(registry, position, 5.0f, 0.5f, WEAPON_TYPE::SWORD);
        }
        attacker.weapon = enemy_weapon;

        // Use circle collider for enemy
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_projectile(Registry& registry, Motion& attacker_motion, Attacker& attacker, Weapon& weapon, TEAM_ID team_id) {
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

        if (projectile_model && projectile_model->mesh_list.size() > 0) {
            // Find mesh with most triangles
            const Mesh* best_mesh = nullptr;
            size_t max_triangles = 0;
            
            // Added logging because hard to debug mesh-based collisions; we can remove later
            // Log::log_info(std::string("Processing meshes for ") + 
            //               (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") + 
            //               " projectile. Total meshes: " + std::to_string(projectile_model->mesh_list.size()), 
            //               __FILE__, __LINE__);
            
            for (size_t i = 0; i < projectile_model->mesh_list.size(); i++) {
                const Mesh* current_mesh = projectile_model->mesh_list[i].get();
                size_t triangle_count = current_mesh->triangles.size();
                
                if (triangle_count > max_triangles) {
                    max_triangles = triangle_count;
                    best_mesh = current_mesh;
                }
            }
            
            if (!best_mesh || max_triangles == 0) {
                Log::log_warning(std::string("No valid mesh found for ") + 
                                (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") + 
                                " projectile, falling back to circle collider", 
                                __FILE__, __LINE__);
                registry.collision_bounds.emplace(entity,
                    CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
                return entity;
            }

            const Mesh* mesh = best_mesh;
            
            // Create transformation matrices
            glm::mat4 pre_transform = projectile_model->get_pre_transform();
            
            // Scale from model's inherent scale
            glm::vec3 model_scale = projectile_model->get_scale();
            
            // Create model matrix for initial orientation (without position)
            glm::mat4 model_matrix = Transform::create_model_matrix(
                glm::vec3(0),           // Position will be handled by physics system
                glm::vec3(0, 0, motion.angle),  // Only rotate around Z axis
                model_scale             // Use model's scale
            );
            
            // Transform vertices through model space to world space
            std::vector<glm::vec2> vertices_2d;
            auto add_unique_vertex = [&vertices_2d](const glm::vec2& v) {
                const float EPSILON = 0.0001f;
                for (const auto& existing : vertices_2d) {
                    if (glm::length(existing - v) < EPSILON) return;
                }
                vertices_2d.push_back(v);
            };

            // Transform all vertices through the combined transformation
            glm::mat4 transform = model_matrix * pre_transform;
            for (const Triangle& tri : mesh->triangles) {
                // Transform vertices to world space (excluding position)
                glm::vec4 v0 = transform * glm::vec4(tri.v0, 1.0f);
                glm::vec4 v1 = transform * glm::vec4(tri.v1, 1.0f);
                glm::vec4 v2 = transform * glm::vec4(tri.v2, 1.0f);
                
                // Project to 2D (XY plane) and add unique vertices
                add_unique_vertex(glm::vec2(v0.x, v0.y));
                add_unique_vertex(glm::vec2(v1.x, v1.y));
                add_unique_vertex(glm::vec2(v2.x, v2.y));
            }
            
            if (vertices_2d.empty()) {
                Log::log_warning(std::string("No valid vertices after transformation for ") + 
                                (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") + 
                                " projectile, falling back to circle collider", 
                                __FILE__, __LINE__);
                registry.collision_bounds.emplace(entity,
                    CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
                return entity;
            }
            
            // Compute centroid for sorting
            glm::vec2 centroid(0.0f);
            for (const auto& v : vertices_2d) {
                centroid += v;
            }
            centroid /= static_cast<float>(vertices_2d.size());
            
            // Sort vertices counter-clockwise around centroid
            std::sort(vertices_2d.begin(), vertices_2d.end(),
                [centroid](const glm::vec2& a, const glm::vec2& b) {
                    return atan2(a.y - centroid.y, a.x - centroid.x) <
                           atan2(b.y - centroid.y, b.x - centroid.x);
                });
            
            // Calculate bound radius for broad-phase collision
            float bound_radius = 0.0f;
            for (const auto& vertex : vertices_2d) {
                bound_radius = std::max(bound_radius, glm::length(vertex - centroid));
            }
            
            // Added logging because hard to debug mesh-based collisions; we can remove later
            // Log::log_success(std::string("Created ") + 
            //                  (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") + 
            //                  " projectile collider with " + std::to_string(vertices_2d.size()) + 
            //                  " vertices and radius " + std::to_string(bound_radius), 
            //                  __FILE__, __LINE__);
            
            registry.collision_bounds.emplace(entity,
                CollisionBounds::create_mesh(vertices_2d, bound_radius));
        } else {
            // Added logging because hard to debug mesh-based collisions; we can remove later
            Log::log_warning(std::string("No valid mesh found for ") + 
                                (weapon.projectile_type == PROJECTILE_TYPE::ARROW ? "ARROW" : "MELEE") + 
                                " projectile, falling back to circle collider", 
                                __FILE__, __LINE__);
            registry.collision_bounds.emplace(entity,
                CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));
        }

        return entity;
    }

    inline Entity create_wall(Registry& registry, glm::vec2 position, float angle, glm::vec2 scale = glm::vec2(2.0f, 2.0f)) {
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
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_wall(motion.scale, motion.angle));

        return entity;
    }

    inline Entity create_no_collision_wall(Registry& registry, glm::vec2 position, float angle, glm::vec2 scale = glm::vec2(2.0f, 2.0f)) {
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

    inline Entity create_tree(Registry& registry, glm::vec2 position, float angle = 0.0f) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(4.0f, 4.0f);
        motion.angle = angle;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& tree = registry.static_objects.emplace(entity);
        tree.type = STATIC_OBJECT_TYPE::TREE;

        // Use circle collider for tree
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_rock(Registry& registry, glm::vec2 position) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(13.0f);

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& tree = registry.static_objects.emplace(entity);
        tree.type = STATIC_OBJECT_TYPE::ROCK;

        // Use circle collider for tree
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_bonfire(Registry& registry, glm::vec2 position) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.5f);

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& obj = registry.static_objects.emplace(entity);
        obj.type = STATIC_OBJECT_TYPE::BONFIRE;

        // Use circle collider for tree
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        auto& interact = registry.interactables.emplace(entity);
        interact.entity = entity;
        interact.range = 5.0f;
        interact.type = INTERACTABLE_TYPE::BONFIRE;

        LightSource& light_source = registry.light_sources.emplace(entity);
        light_source.pos = glm::vec3(position, 1.0f);
        light_source.brightness = 10.0f;
        light_source.colour = glm::vec3(252.0f/255.0f, 116.0f/255.0f, 5.0f/255.0f);

        return entity;
    }

    // needs new static type
    inline Entity create_portal(Registry& registry, glm::vec2 position, INTERACTABLE_TYPE type, int dungeon_difficulty = 0) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.5f);

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::NEUTRAL;

        auto& tree = registry.static_objects.emplace(entity);
        tree.type = STATIC_OBJECT_TYPE::PORTAL;

        registry.collision_bounds.emplace(entity,
           CollisionBounds::create_wall({0.1, 7.7}, 0));

        auto& interact = registry.interactables.emplace(entity);
        interact.entity = entity;
        interact.range = 5.0f;
        interact.type = type;
        interact.dungeon_difficulty = dungeon_difficulty;

        auto entity_l1 = Entity();
        auto& light_source1 = registry.light_sources.emplace(entity_l1);
        light_source1.brightness = 5.0f;
        light_source1.colour = glm::vec3(103.f/255.f,0.f/255.f,116.f/255.f);
        light_source1.pos = glm::vec3(position.x - 0.2, position.y, 2.0f);
        auto entity_l2 = Entity();
        auto& light_source2 = registry.light_sources.emplace(entity_l2);
        light_source2.brightness = 5.0f;
        light_source2.colour = glm::vec3(103.f/255.f,0.f/255.f,116.f/255.f);
        light_source2.pos = glm::vec3(position.x + 0.2, position.y, 2.0f);

        return entity;
    }

    inline Entity create_light_source(Registry& registry, glm::vec3 position, float brightness, glm::vec3 colour, LIGHT_SOURCE_TYPE type = LIGHT_SOURCE_TYPE::LIGHT_SOURCE_TYPE_COUNT) {
        Entity e = Entity();
        LightSource& light_source = registry.light_sources.emplace(e);
        light_source.pos = position;
        light_source.brightness = brightness;
        light_source.colour = colour;
        light_source.type = type;
        return e;
    }

    inline Entity create_boss_projectile(Registry& registry, glm::vec2 pos, float angle, glm::vec2 aim, Weapon& weapon) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = pos;
        motion.angle = angle;
        motion.velocity = aim * weapon.proj_speed;
        motion.scale = glm::vec2(1.0f, 1.0f);  // Projectile size

        auto& projectile = registry.projectiles.emplace(entity);
        projectile.damage = weapon.damage;
        projectile.range_remaining = weapon.range;
        projectile.stagger_duration = weapon.stagger_duration;
        projectile.poise_points = weapon.poise_points;
        projectile.enchantment = ENCHANTMENT::NONE;
        projectile.projectile_type = weapon.projectile_type;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        registry.collision_bounds.emplace(entity,
                CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }



    inline Entity create_test_boss(Registry& registry, glm::vec2 position) {
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(3.0f, 3.0f);  // Enemy size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.max_health = 100.0f;
        locomotion.health = locomotion.max_health;
        locomotion.max_energy = 1000.0f;
        locomotion.energy = locomotion.max_energy;
        locomotion.max_poise = 1000.0f;
        locomotion.poise = locomotion.max_poise;
        locomotion.movement_speed = 12.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        auto& attacker = registry.attackers.emplace(entity);

        // COMBOS
        BossAI& ai = registry.boss_ais.emplace(entity);
        ai.dodge_ratio = 1.0f;
        ai.attack_range = 6.5f;
        AttackCombo combo1 = {
            .attacks = {BOSS_ATTACK_TYPE::REGULAR, BOSS_ATTACK_TYPE::LONG, BOSS_ATTACK_TYPE::REGULAR},
            .delays = {0.0f, 0.5f, 0.3f}
        };
        AttackCombo combo2 = {
            .attacks = {BOSS_ATTACK_TYPE::AOE, BOSS_ATTACK_TYPE::LONG, BOSS_ATTACK_TYPE::AOE},
            .delays = {0.0f, 0.5f, 0.4f}
        };
        AttackCombo combo3 = {
            .attacks = {BOSS_ATTACK_TYPE::REGULAR, BOSS_ATTACK_TYPE::REGULAR, BOSS_ATTACK_TYPE::AOE},
            .delays = {0.0f, 0.1f, 0.7f}
        };
        ai.combos = {combo1, combo2, combo3};


        auto& enemy = registry.enemies.emplace(entity);
        enemy.type = ENEMY_TYPE::WARRIOR;

        Entity enemy_weapon = EntityFactory::create_weapon(registry, position, 10.0f, 0.5f, WEAPON_TYPE::SWORD);
        attacker.weapon = enemy_weapon;

        // Use circle collider for enemy
        registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }
};
