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
