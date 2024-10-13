#pragma once

#include <ecs/Registry.hpp>
#include <ecs/Entity.hpp>
#include <vector>

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

	Registry& m_registry;
};
