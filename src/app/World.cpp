#include "World.hpp"
#include "EntityFactory.hpp"
#include "utils/Log.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "InputManager.hpp"

World::World() : m_registry(Registry::get_instance()), m_collisionSystem(), m_physicsSystem() {}

World::~World() = default;

void World::demo_init() {
    // Create Player
    auto player = EntityFactory::create_player(glm::vec2(400.0f, 300.0f));
    auto weapon = EntityFactory::create_weapon(glm::vec2(400.0f, 300.0f), 10.0f);
    m_registry.attackers.get(player).weapon_id = weapon;
    m_players.push_back(player);

    // Create some enemies
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(100.0f + i * 100.0f, 100.0f);
        auto enemy = EntityFactory::create_enemy(pos);
        auto enemy_weapon = EntityFactory::create_weapon(pos, 5.0f);
        m_registry.attackers.get(enemy).weapon_id = enemy_weapon;
        m_enemies.push_back(enemy);
    }
}

void World::step(float elapsed_ms) {
    // TODO: Update the game world
    // 1. Update physics
    m_physicsSystem.step(elapsed_ms);
    
    // 2. Check and handle collisions
    m_collisionSystem.check_collisions();
    handle_collisions();
  
    InputManager::handle_inputs_per_frame();

    // update attack cooldown comps
    for (Entity& e : m_registry.attack_cooldowns.entities) {
        auto& attack_cooldown = m_registry.attack_cooldowns.get(e);
        attack_cooldown.timer -= elapsed_ms;
        if (attack_cooldown.timer <= 0) {
            m_registry.attack_cooldowns.remove(e);
        }
    }
}

void World::handle_collisions() {
    auto& collisions = m_registry.collisions;
    for (uint i = 0; i < collisions.components.size(); i++) {
        // The entity and its collider
        Entity entity = collisions.entities[i];
        Entity entity_other = collisions.components[i].other;

        if (m_registry.teams.has(entity) && m_registry.teams.has(entity_other)) {
            auto& team1 = m_registry.teams.get(entity);
            auto& team2 = m_registry.teams.get(entity_other);

            if (team1.team_id != team2.team_id) {
                if (m_registry.projectile_stats.has(entity)) {
                    handle_projectile_collision(entity, entity_other);
                } else if (m_registry.projectile_stats.has(entity_other)) {
                    handle_projectile_collision(entity_other, entity);
                } else {
                    handle_entity_collision(entity, entity_other);
                }
            }
        }

        m_registry.collisions.remove(entity);
    }
}

void World::handle_projectile_collision(const Entity& projectile, const Entity& target) {
    auto& projectile_stats = m_registry.projectile_stats.get(projectile);
    auto& target_stats = m_registry.locomotion_stats.get(target);

    target_stats.health -= projectile_stats.damage;
    Log::log_info("Projectile hit target. Target health: " + std::to_string(target_stats.health), __FILE__, __LINE__);

    m_registry.remove_all_components_of(projectile);

    if (target_stats.health <= 0) {
        if (m_registry.teams.get(target).team_id == TEAM_ID::FOW) {
            m_registry.remove_all_components_of(target);
            Log::log_info("Enemy defeated", __FILE__, __LINE__);
        }
    }
}

void World::handle_entity_collision(const Entity& entity1, const Entity& entity2) {
    auto& motion1 = m_registry.motions.get(entity1);
    auto& motion2 = m_registry.motions.get(entity2);

    glm::vec2 direction = glm::normalize(motion1.position - motion2.position);
    float push_distance = 5.0f;

    motion1.position += direction * push_distance;
    motion2.position -= direction * push_distance;

    Log::log_info("Entities collided and were pushed apart", __FILE__, __LINE__);
}
