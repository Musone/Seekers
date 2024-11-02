#pragma once

#include <glm/vec2.hpp>

enum class PROJECTILE_TYPE
{
    ARROW = 0,
    MELEE = ARROW + 1,
    MAGIC = MELEE + 1,
    ATTACK_STYLE_COUNT = MAGIC + 1
};

const int projectile_type_count = (int)PROJECTILE_TYPE::ATTACK_STYLE_COUNT;

enum class WEAPON_TYPE
{
    PUNCH = 0,
    SWORD = 1,
    BOW = SWORD + 1,
    WAND = BOW + 1,
    WEAPON_TYPE_COUNT = WAND + 1
};

enum class ENCHANTMENT
{
    NONE = 0,
    FIRE = NONE + 1,
    ICE = FIRE + 1,
    ENCHANTMENT_COUNT = ICE + 1
};

struct Weapon
{
    WEAPON_TYPE type = WEAPON_TYPE::WEAPON_TYPE_COUNT;
    float damage;
    float range;
    float proj_speed;
    float attack_cooldown;
    float stagger_duration;
    float poise_points;
    float attack_energy_cost;
    PROJECTILE_TYPE projectile_type = PROJECTILE_TYPE::ATTACK_STYLE_COUNT;
    ENCHANTMENT enchantment = ENCHANTMENT::ENCHANTMENT_COUNT;
};

struct Attacker
{
    glm::vec2 aim = {0, 0}; // it is normalized
    unsigned int weapon_id;
};

struct AttackCooldown
{
    float timer;
    AttackCooldown(float t) : timer(t) {}
};

struct StaggerCooldown
{
    float timer;
    StaggerCooldown(float t) : timer(t) {}
};

struct DeathCooldown {
    float timer;
    DeathCooldown(float t) : timer(t) {}
};

struct EnergyNoRegenCooldown {
    float timer;
    EnergyNoRegenCooldown(float t) : timer(t) {}
};

struct Projectile
{
    float damage;
    float range_remaining;
    float stagger_duration;
    float poise_points;
    PROJECTILE_TYPE projectile_type;
    ENCHANTMENT enchantment; // Added for future milestones
};

enum class TEAM_ID
{
	FRIENDLY = 0,
	FOW = FRIENDLY + 1,
	NEUTRAL = FOW + 1,
	TEAM_ID_COUNT = NEUTRAL + 1
};

const int team_id_count = (int)TEAM_ID::TEAM_ID_COUNT;

struct Team
{
	TEAM_ID team_id = TEAM_ID::TEAM_ID_COUNT;
};
