#pragma once

#include "../ecs/Registry.hpp"
#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include <glm/glm.hpp>

class WorldInit {
public:
    static Entity create_player(glm::vec2 position);
    static Entity create_enemy(glm::vec2 position);
    static Entity create_projectile(glm::vec2 position, glm::vec2 velocity, TEAM_ID team_id);

private:
    static Registry& registry;
};
