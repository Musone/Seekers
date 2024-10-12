#include "World.hpp"
#include "utils/Log.hpp"

World::World() : m_registry(), m_collisionSystem(), m_physicsSystem() {}

World::~World() = default;

void World::init()
{
    // TODO: Initialize the game world
    // 1. Create initial entities (player, enemies, etc.)
    // 2. Set up any necessary systems or resources
}

void World::step(float elapsed_ms)
{
    // TODO: Update the game world
    // 1. Update physics
    m_physicsSystem.step(m_registry, elapsed_ms);

    // 2. Check for collisions
    m_collisionSystem.check_collisions();

    // 3. Handle collisions
    handle_collisions();

    // 4. Update other game logic (AI, buffs, etc.)
}

void World::handle_collisions()
{
    // TODO: Implement collision handling logic
    // 1. Iterate through entities with Collision components
    // 2. Handle different types of collisions (e.g., projectile-enemy, player-enemy)
}

void World::handle_projectile_collisions(Entity projectile, Entity target)
{
    // TODO: Implement projectile collision handling
    // 1. Check if the projectile and target are on different teams
    // 2. Apply damage to the target
    // 3. Remove the projectile entity
}

void World::handle_player_enemy_collisions(Entity player, Entity enemy)
{
    // TODO: Implement player-enemy collision handling
}

void World::apply_damage(Entity attacker, Entity target, float damage)
{
    // TODO: Implement damage application
    // 1. Reduce target's health based on damage and defense
    // 2. Check if the target is defeated
    // 3. Handle any on-hit effects or buffs
}