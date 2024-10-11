#pragma once

#include <ecs/Entity.hpp>

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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

struct Attacker
{
	glm::vec2 aim = {0, 0}; // it is normalized
	unsigned int weapon_id;
};

struct LocomotionStats
{
	float health;
	float max_health;
	float energy;
	float max_energy;
	float defense;
	float power;
	float agility;
	float movement_speed;
};

enum class BUFF_EFFECT
{
	GLOW = 0,
	ENLARGE = GLOW + 1,
	BUFF_EFFECT_COUNT = ENLARGE + 1
};
const int buff_effect_count = (int)BUFF_EFFECT::BUFF_EFFECT_COUNT;

struct Buff
{
	float remaining_time;
	float health;
	float energy;
	float defense;
	float power;
	float agility;
	float movement_speed;
	BUFF_EFFECT effect = BUFF_EFFECT::BUFF_EFFECT_COUNT;
};

enum class ATTACK_STYLE
{
	ONE_AIM = 0,
	TWO_AIM = ONE_AIM + 1,
	THREE_WIDE = TWO_AIM + 1,
	EIGHT_ALL_DIR = THREE_WIDE + 1,
	ATTACK_STYLE_COUNT = EIGHT_ALL_DIR + 1
};
const int attack_style_count = (int)ATTACK_STYLE::ATTACK_STYLE_COUNT;

struct WeaponStats
{
	float damage;
	float range;
	float proj_speed;
	float attack_cooldown;
	ATTACK_STYLE attack_style = ATTACK_STYLE::ATTACK_STYLE_COUNT;
};

struct ProjectileStats
{
	float damage;
	float range_remaining;
};

struct Collision
{
	Entity other;
	Collision(Entity &other) { this->other = other; };
};

struct AttackCooldown {
	float timer;
};