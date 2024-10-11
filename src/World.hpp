#pragma once

#include <ecs/Registry.hpp>
#include <ecs/Entity.hpp>
#include <systems/CollisionSystem.hpp>
#include <systems/PhysicsSystem.hpp>
// stlib
#include <vector>
#include <random>

#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include <glm/vec2.hpp>

class World {
public:
	World();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	// void init(RenderSystem* renderer);

	// Releases all associated resources
	~World();

	// Initialize the game world
    void init();

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over() const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(glm::vec2 pos);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	// RenderSystem* renderer;
	float current_speed;
	float next_eel_spawn;
	float next_fish_spawn;
	Entity player_salmon;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// ECS registry
	Registry m_registry;

	// Collision system
	CollisionSystem m_collisionSystem;

	// Physics system
	PhysicsSystem m_physicsSystem;

	// Helper functions for specific collision handling
	void handle_projectile_collisions(Entity projectile, Entity target);
	void handle_player_enemy_collisions(Entity player, Entity enemy);
	void apply_damage(Entity attacker, Entity target, float damage);
};