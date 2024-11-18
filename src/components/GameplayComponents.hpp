#pragma once

#include <vector>
#include "CombatComponents.hpp"

struct LocomotionStats
{
    float health;
    float max_health;
    float energy;
    float max_energy;
    float poise;
    float max_poise;
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

// Added for future milestones
struct InventoryItem
{
    unsigned int item_id;
    unsigned int count;
};

// Added for future milestones
struct Inventory
{
    std::vector<InventoryItem> items;
    unsigned int capacity;
};

// Added for future milestones
struct Equipment
{
    Weapon equipped_weapon;
    unsigned int armor_id;
};

// Added for future milestones
enum class WEATHER_TYPE
{
	SUNNY = 0,
	RAINY = SUNNY + 1,
	SNOWING = RAINY + 1,
	FOGGY = SNOWING + 1,
	WEATHER_TYPE_COUNT = FOGGY + 1
};

const int weather_type_count = (int)WEATHER_TYPE::WEATHER_TYPE_COUNT;

// Added for future milestones
struct WeatherState
{
    WEATHER_TYPE current_weather;
    float intensity;
    float duration_remaining;
};

// Added for future milestones
enum class INTERACTABLE_TYPE
{
    ITEM_PICKUP = 0,
    DUNGEON_ENTRANCE = ITEM_PICKUP + 1,
    DUNGEON_EXIT = DUNGEON_ENTRANCE + 1,
    SPIRE_ENTRANCE = DUNGEON_EXIT + 1,
    NPC = SPIRE_ENTRANCE + 1,
    INTERACTABLE_TYPE_COUNT = NPC + 1
};

const int interactable_type_count = (int)INTERACTABLE_TYPE::INTERACTABLE_TYPE_COUNT;

// Added for future milestones
struct Interactable
{
    INTERACTABLE_TYPE type;
    Entity entity;
    float range;
};

struct InDodge {
    glm::vec2 source;
    glm::vec2 destination;
    float origin_time;
    float duration;
    InDodge(glm::vec2 source, glm::vec2 destination, float origin_time, float duration) : source(source), destination(destination), origin_time(origin_time), duration(duration) {}
};

struct NearPlayer {

};

struct NearCamera {

};
