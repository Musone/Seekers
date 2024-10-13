#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include <glm/glm.hpp>

namespace WorldInit {
    Entity create_player(glm::vec2 position) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(50.0f, 50.0f);  // Player size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 100.0f;
        locomotion.max_health = 100.0f;
        locomotion.movement_speed = 200.0f;
        locomotion.energy = 100.0f;
        locomotion.max_energy = 100.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FRIENDLY;

        // Create a weapon entity
        auto weapon_entity = Entity();
        auto& weapon_stats = registry.weapon_stats.emplace(weapon_entity);
        weapon_stats.damage = 10.0f;
        weapon_stats.range = 100.0f;
        weapon_stats.proj_speed = 300.0f;
        weapon_stats.attack_cooldown = 1.0f;
        weapon_stats.attack_style = ATTACK_STYLE::ONE_AIM;

        // Create and set up the Attacker component for the player
        auto& attacker = registry.attackers.emplace(entity);
        attacker.weapon_id = weapon_entity;

        return entity;
    }

    Entity create_enemy(glm::vec2 position) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(40.0f, 40.0f);  // Enemy size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 50.0f;
        locomotion.max_health = 50.0f;
        locomotion.movement_speed = 100.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        // Create a weapon entity for the enemy
        auto weapon_entity = Entity();
        auto& weapon_stats = registry.weapon_stats.emplace(weapon_entity);
        weapon_stats.damage = 5.0f;
        weapon_stats.range = 50.0f;
        weapon_stats.proj_speed = 200.0f;
        weapon_stats.attack_cooldown = 2.0f;
        weapon_stats.attack_style = ATTACK_STYLE::ONE_AIM;

        // Create and set up the Attacker component for the enemy
        auto& attacker = registry.attackers.emplace(entity);
        attacker.weapon_id = weapon_entity;

        return entity;
    }

    Entity create_projectile(glm::vec2 position, glm::vec2 velocity, TEAM_ID team_id) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.velocity = velocity;
        motion.scale = glm::vec2(10.0f, 10.0f);  // Projectile size

        auto& projectile = registry.projectile_stats.emplace(entity);
        projectile.damage = 10.0f;
        projectile.range_remaining = 100.0f;
        projectile.enchantment = ENCHANTMENT::NONE;

        auto& team = registry.teams.emplace(entity);
        team.team_id = team_id;

        return entity;
    }
};
