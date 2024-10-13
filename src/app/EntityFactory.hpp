#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include <glm/glm.hpp>

namespace EntityFactory {
    inline Entity create_player(glm::vec2 position) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.0f, 1.0f);  // Player size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 100.0f;
        locomotion.max_health = 100.0f;
        locomotion.movement_speed = 5.0f;
        locomotion.energy = 100.0f;
        locomotion.max_energy = 100.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FRIENDLY;

        registry.attackers.emplace(entity);

        auto& texture = registry.textures.emplace(entity);
        texture.name = "player.png";

        auto& bounding_box = registry.bounding_boxes.emplace(entity);
        bounding_box.radius = 11.3f;

        return entity;
    }

    inline Entity create_weapon(glm::vec2 position, float damage, unsigned int following) {
        Registry& registry = Registry::get_instance();

        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;

        auto weapon_entity = Entity();
        auto& weapon_stats = registry.weapon_stats.emplace(weapon_entity);
        weapon_stats.damage = damage;
        weapon_stats.range = 100.0f;
        weapon_stats.proj_speed = 300.0f;
        weapon_stats.attack_cooldown = 1.0f;
        weapon_stats.attack_style = ATTACK_STYLE::ONE_AIM;

        registry.move_withs.emplace(entity, following);
        registry.rotate_withs.emplace(entity, following);

        auto& texture = registry.textures.emplace(entity);
        texture.name = "sword_2.png";

        return entity;
    }

    inline Entity create_enemy(glm::vec2 position) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.scale = glm::vec2(1.0f, 1.0f);  // Enemy size

        auto& locomotion = registry.locomotion_stats.emplace(entity);
        locomotion.health = 50.0f;
        locomotion.max_health = 50.0f;
        locomotion.movement_speed = 100.0f;

        auto& team = registry.teams.emplace(entity);
        team.team_id = TEAM_ID::FOW;

        registry.attackers.emplace(entity);

        registry.rotate_withs.emplace(entity, registry.player);

        auto& texture = registry.textures.emplace(entity);
        texture.name = "skeleton.png";

        auto& bounding_box = registry.bounding_boxes.emplace(entity);
        bounding_box.radius = 11.3f;

        return entity;
    }

    inline Entity create_projectile(glm::vec2 position, Attacker& attacker, WeaponStats& weapon, TEAM_ID team_id) {
        Registry& registry = Registry::get_instance();
        auto entity = Entity();

        auto& motion = registry.motions.emplace(entity);
        motion.position = position;
        motion.angle = atan2(attacker.aim.y, attacker.aim.x);
        motion.velocity = attacker.aim * weapon.proj_speed;;
        motion.scale = glm::vec2(0.025f, 0.025f);  // Projectile size

        auto& projectile = registry.projectile_stats.emplace(entity);
        projectile.damage = weapon.damage;
        projectile.range_remaining = weapon.range;
        projectile.enchantment = ENCHANTMENT::NONE;

        auto& team = registry.teams.emplace(entity);
        team.team_id = team_id;

        auto& texture = registry.textures.emplace(entity);
        texture.name = "projectile.png";

        auto& bounding_box = registry.bounding_boxes.emplace(entity);
        bounding_box.radius = 144.0f;

        return entity;
    }
};
