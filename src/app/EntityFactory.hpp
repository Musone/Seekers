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
        locomotion.movement_speed = 25.0f;
        locomotion.energy = 100.0f;
        locomotion.max_energy = 100.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FRIENDLY;

        registry.attackers.emplace(entity);

        // Use circle collider for player
        auto& bounds = registry.collision_bounds.emplace(entity,
            CollisionBounds::create_circle(Common::max_of(motion.scale) / 2));

        return entity;
    }

    inline Entity create_weapon(glm::vec2 position, float damage, float attack_cooldown = 0.15f) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.5f, 1.5f);

        auto& weapon = registry.weapons.emplace(entity);
        weapon.type = WEAPON_TYPE::SWORD;
        weapon.damage = damage;
        weapon.range = 30.0f;
        weapon.proj_speed = 100.0f;
        weapon.attack_cooldown = attack_cooldown;
        weapon.attack_style = ATTACK_STYLE::ONE_AIM;
        weapon.enchantment = ENCHANTMENT::NONE;

        return entity;
    }

    inline Entity create_enemy(glm::vec2 position) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(3.0f, 3.0f);  // Enemy size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 50.0f;
        locomotion.max_health = 50.0f;
        locomotion.movement_speed = 5.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        registry.attackers.emplace(entity);

        AIComponent& ai = registry.ais.emplace(entity);
        ai.current_state = AI_STATE::PATROL;
        ai.patrol_points.push_back(position);
        ai.patrol_points.push_back(position + glm::vec2(10.0f, 0.0f));
        ai.target_position = position;

        auto& enemy = registry.enemies.emplace(entity);
        enemy.type = ENEMY_TYPE::WARRIOR;

        auto enemy_weapon = EntityFactory::create_weapon(position, 5.0f);
        registry.attackers.get(entity).weapon_id = enemy_weapon;

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

        auto& projectile = registry.projectile_stats.emplace(entity);
        projectile.damage = weapon.damage;
        projectile.range_remaining = weapon.range;
        projectile.enchantment = ENCHANTMENT::NONE;

        auto& team = registry.teams.emplace(entity);
        team.team_id = team_id;

        auto& texture = registry.textures.emplace(entity);
        texture.name = "projectile.png";

        // Test: Mesh based collider for an arrow/projectile shape
        // TODO: Replace with actual projectile model when we have one
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
            CollisionBounds::create_wall(motion.scale));

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
