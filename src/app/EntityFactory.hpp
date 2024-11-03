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
            const Mesh* mesh = projectile_model->mesh_list[0].get();
            std::vector<glm::vec2> vertices_2d;
            
            // Convert 3D mesh triangles to 2D vertices for collision
            for (const Triangle& tri : mesh->triangles) {
                // Apply pre-transform from static model to vertices
                glm::vec4 v0 = projectile_model->get_model_matrix() * projectile_model->get_pre_transform() * glm::vec4(tri.v0, 1.0f);
                glm::vec4 v1 = projectile_model->get_model_matrix() * projectile_model->get_pre_transform() * glm::vec4(tri.v1, 1.0f);
                glm::vec4 v2 = projectile_model->get_model_matrix() * projectile_model->get_pre_transform() * glm::vec4(tri.v2, 1.0f);
                
                vertices_2d.push_back(glm::vec2(v0.x, v0.y));
                vertices_2d.push_back(glm::vec2(v1.x, v1.y));
                vertices_2d.push_back(glm::vec2(v2.x, v2.y));
            }

            float cos_angle = cos(motion.angle);
            float sin_angle = sin(motion.angle);
            for (auto& vertex : vertices_2d) {
                float x = vertex.x;
                float y = vertex.y;
                vertex.x = x * cos_angle - y * sin_angle;
                vertex.y = x * sin_angle + y * cos_angle;
            }

            auto& bounds = registry.collision_bounds.emplace(entity,
                CollisionBounds::create_mesh(vertices_2d, Common::max_of(motion.scale)));
        } else {
            // Fallback to default arrow shape if no model available
            // Define arrow/projectile shape
            std::vector<glm::vec2> vertices = {
                // Define arrow/projectile shape
                {-0.5f, -0.1f},  // back left
                {-0.3f, -0.2f},  // wing left
                {0.5f, 0.0f},    // tip
                {-0.3f, 0.2f},   // wing right
                {-0.5f, 0.1f}    // back right
            };

            // Scale vertices
            for (auto& vertex : vertices) {
                vertex *= motion.scale;
            }

            // Rotate vertices based on motion.angle
            float cos_angle = cos(motion.angle);
            float sin_angle = sin(motion.angle);
            for (auto& vertex : vertices) {
                float x = vertex.x;
                float y = vertex.y;
                vertex.x = x * cos_angle - y * sin_angle;
                vertex.y = x * sin_angle + y * cos_angle;
            }

            auto& bounds = registry.collision_bounds.emplace(entity,
                CollisionBounds::create_mesh(vertices, Common::max_of(motion.scale)));
        }

        // Circle collider will also work
        // auto& bounds = registry.collision_bounds.emplace(entity,
        //     CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

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
