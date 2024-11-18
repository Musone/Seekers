#pragma once

namespace InteractionSystem {
    inline void update_near_interactable() {
        Registry& registry = MapManager::get_instance().get_active_registry();
        Motion& player_motion = registry.motions.get(registry.player);

        std::vector<Entity> in_range_interactables;
        for (Entity& e : registry.near_players.entities) {
            if (registry.interactables.has(e)) {
                if (!registry.motions.has(e)) continue;
                glm::vec2 pos = registry.motions.get(e).position;
                if (glm::distance(player_motion.position, pos) < registry.interactables.get(e).range) {
                    if (Common::get_angle_between_item_and_player_view(pos, player_motion.position, player_motion.angle) < Globals::interactable_angle) {
                        in_range_interactables.push_back(e);
                    }
                }
            }
        }

        if (in_range_interactables.size() == 0) {
            registry.near_interactable.is_active = false;
            return;
        }
        registry.near_interactable.is_active = true;

        // pick the best in range interactable (add priority and other thing here. for now it's just whichever closer)
        float min_distance = std::numeric_limits<float>::max();
        for (Entity& e : in_range_interactables) {
            glm::vec2 pos = registry.motions.get(e).position;
            float distance = glm::distance(pos, player_motion.position);
            if (distance < min_distance) {
                registry.near_interactable.interactable = e;
                min_distance = distance;
            }
        }
    }

    inline void interact() {
        Registry& registry = MapManager::get_instance().get_active_registry();

        if (!registry.near_interactable.is_active) return;

        Entity& interactable = registry.near_interactable.interactable;
        Interactable& comp = registry.interactables.get(interactable);
        if (comp.type == INTERACTABLE_TYPE::DUNGEON_ENTRANCE) {
            MapManager::get_instance().enter_dungeon_flag = true;
        } else if (comp.type == INTERACTABLE_TYPE::DUNGEON_EXIT) {
            MapManager::get_instance().return_open_world_flag = true;
        } else if (comp.type == INTERACTABLE_TYPE::BONFIRE) {
            GameplaySystem::rest();
            // save the game here or inside rest function
        }
    }
};