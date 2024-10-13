#pragma once

#include <glm/vec2.hpp>
#include "ecs/Entity.hpp"

struct Motion
{
	glm::vec2 position = {0, 0};
	float angle = 0;
	float rotation_velocity = 0;
	glm::vec2 scale = {0, 0};
	glm::vec2 velocity = {0, 0};
	glm::vec2 acceleration = {0, 0};
	float drag = 0;
};

struct MoveWith
{
	unsigned int following_entity_id;
	MoveWith(unsigned int id) : following_entity_id(id) {}
};

struct RotateWith
{
	unsigned int following_entity_id;
	RotateWith(unsigned int id) : following_entity_id(id) {}
};

struct Collision
{
	Entity other;
	Collision(Entity &other) { this->other = other; };
};
