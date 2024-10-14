#include "World.h"
#include "EntityFactory.hpp"

#include <systems/CollisionSystem.hpp>
#include <app/InputManager.hpp>

#include "systems/GameplaySystem.hpp"
#include "systems/PhysicsSystem.hpp"

#include <components/RenderComponents.hpp> // For Motion component

World::World() : m_registry(Registry::get_instance()){}

World::~World() = default;

void World::demo_init() {
    // Create Player
    auto player = EntityFactory::create_player(glm::vec2(0.0f, 0.0f));
    auto weapon = EntityFactory::create_weapon(glm::vec2(10.0f, 5.0f), 10.0f, player);
    m_registry.attackers.get(player).weapon_id = weapon;
    m_players.push_back(player);
    m_registry.player = player;

    // Create some enemies
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(10.0f + i * 10.0f, 10.0f);
        auto enemy = EntityFactory::create_enemy(pos);
        auto enemy_weapon = EntityFactory::create_weapon(pos, 5.0f, enemy);
        m_registry.attackers.get(enemy).weapon_id = enemy_weapon;
        m_enemies.push_back(enemy);
    }

    // Create some walls
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(5.0f + i * 2.0f, 5.0f);
        EntityFactory::create_wall(pos, 0.0f);
    }
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(5.0f, (i + 1) * 2.0f + 5.0f);
        EntityFactory::create_wall(pos, PI / 2.0f);
    }

    // Place a tree
    EntityFactory::create_tree(glm::vec2(15.0f, -15.0f));
}

void World::step(float elapsed_ms) {
    // TODO: Update the game world
    // 1. Update physics
    PhysicsSystem::step(elapsed_ms);

    CollisionSystem::check_collisions();
    CollisionSystem::handle_collisions();

    InputManager::handle_inputs_per_frame();

    GameplaySystem::update_cooldowns(elapsed_ms);
    GameplaySystem::update_projectile_range(elapsed_ms);

    // Enforce boundaries for players
    for (const auto& player : m_players) {
        enforce_boundaries(player);
    }
}

void World::enforce_boundaries(Entity entity) {
    if (m_registry.motions.has(entity)) {
        Motion& motion = m_registry.motions.get(entity);
        
        // Adjust these values based on the actual visible boundaries of your map
        const float LEFT_BOUND = -MAP_WIDTH / 2.0f;  // Assuming the map is centered
        const float RIGHT_BOUND = MAP_WIDTH / 2.0f;
        const float TOP_BOUND = MAP_HEIGHT / 2.0f;
        const float BOTTOM_BOUND = -MAP_HEIGHT / 2.0f;

        // Enforce X-axis boundaries
        if (motion.position.x < LEFT_BOUND) {
            motion.position.x = LEFT_BOUND;
        } else if (motion.position.x > RIGHT_BOUND) {
            motion.position.x = RIGHT_BOUND;
        }

        // Enforce Y-axis boundaries
        if (motion.position.y < BOTTOM_BOUND) {
            motion.position.y = BOTTOM_BOUND;
        } else if (motion.position.y > TOP_BOUND) {
            motion.position.y = TOP_BOUND;
        }
    }
}
