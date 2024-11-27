#pragma once

#include <glm/vec2.hpp>
#include <vector>

enum class AI_STATE
{
    IDLE,
    PATROL,
    CHASE,
    ATTACK
};

struct AIComponent
{
    AI_STATE current_state; // Current state of the AI (idle, patrol, chase, or attack)
    glm::vec2 target_position; // Position the AI is targeting (for chase and attack states)
    float detection_radius; // Radius within which the AI can detect entities
    std::vector<glm::vec2> patrol_points; // List of points for the AI to patrol
};

struct VisionToPlayer
{
    float timer;
    VisionToPlayer(float t) : timer(t) {}
};


// boss related stuff down here

enum class BOSS_STATE
{
    IN_COMBO,
    COOLDOWN,
    CHASE
};

std::array<std::string, 3> boss_attack_name = {"regular", "rapid_repeat", "aoe"};

struct AttackCombo {
    std::vector<std::string> attacks;
    std::vector<float> delays;          // optional delay before each attack (same size as attacks list)
};

struct BossAI {
    BOSS_STATE state;
    float cooldown_delay_counter;       // cooldown counter between combos (switch from cooldown state to chase/in_combo)
    float dodge_ratio;                  // between 0 and 1 for the percentage of attacks dodged by boss
    std::vector<AttackCombo> combos;
    std::vector<float> q;               // reward (damage dealt - damage received) list of combos for Q-learning (same size as combos list)
    std::vector<unsigned int> k;        // number of times combo was used for Q-learning (same size as q list)
    unsigned int combo_index;           // index of the current combo (if in IN_COMBO state)
    unsigned int attack_index;          // index of the next attack within current combo
    float attack_delay_counter;         // delay counter before next attack
    float attack_range;                 // distance from player that the boss starts a combo
};