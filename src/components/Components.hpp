#pragma once

#include "src/ecs/Entity.hpp"

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

// Player component
struct Player {

};

struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 scale = { 0, 0 };
	vec2 velocity = { 0, 0 };
	vec2 acceleration = { 0, 0 };
	float drag = 0;
};

struct Collision {
	Entity other;
	Collision(Entity& other) { this->other = other; };
};

// Sets the brightness of the screen
struct ScreenState {
	float darken_screen_factor = -1;
};

// A timer that will be associated to dying salmon
struct DeathTimer {
	float counter_ms = 3000;
};

struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

struct DebugComponent {
	// Note, an empty struct has size 1
};