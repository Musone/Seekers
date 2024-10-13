#pragma once

#include "../ecs/Registry.hpp"

namespace GameplaySystem {
    inline void update_cooldowns(float elapsed_ms) {
        Registry& registry = Registry::get_instance();

        for (Entity& e : registry.attack_cooldowns.entities) {
            auto& attack_cooldown = registry.attack_cooldowns.get(e);
            attack_cooldown.timer -= elapsed_ms;
            if (attack_cooldown.timer <= 0) {
                registry.attack_cooldowns.remove(e);
            }
        }
    }
}