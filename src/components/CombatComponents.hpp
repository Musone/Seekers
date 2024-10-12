#pragma once

#include <glm/vec2.hpp>

enum class ATTACK_STYLE
{
    ONE_AIM = 0,
    TWO_AIM = ONE_AIM + 1,
    THREE_WIDE = TWO_AIM + 1,
    EIGHT_ALL_DIR = THREE_WIDE + 1,
    ATTACK_STYLE_COUNT = EIGHT_ALL_DIR + 1
};

const int attack_style_count = (int)ATTACK_STYLE::ATTACK_STYLE_COUNT;

enum class WEAPON_TYPE
{
    SWORD = 0,
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

struct WeaponStats
{
    float damage;
    float range;
    float proj_speed;
    float attack_cooldown;
    ATTACK_STYLE attack_style = ATTACK_STYLE::ATTACK_STYLE_COUNT;
};

struct Weapon
{
    WEAPON_TYPE type;
    WeaponStats stats;
    ENCHANTMENT enchantment;
};

struct Attacker
{
    glm::vec2 aim = {0, 0}; // it is normalized
    unsigned int weapon_id;
};

struct AttackCooldown
{
    float timer;
};

struct ProjectileStats
{
    float damage;
    float range_remaining;
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
