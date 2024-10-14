#pragma once

#include <ecs/Registry.hpp>
#include <ecs/Entity.hpp>
#include <vector>
#include <globals/Globals.h>
#include <utils/Common.hpp>
#include <systems/AudioSystem.hpp>

class World {
public:
	World();
	~World();

	void demo_init();
	void step(float elapsed_ms);
	void handle_collisions();

	std::vector<Entity> m_players;
	std::vector<Entity> m_enemies;
	std::vector<Entity> m_projectiles;

private:
	void restart_game();
	void handle_projectile_collision(const Entity& projectile, const Entity& target);
	void handle_entity_collision(const Entity& entity1, const Entity& entity2);
	void enforce_boundaries(Entity entity); // Method to enforce boundaries

	Registry& m_registry;
	AudioSystem& m_audioSystem;
};
