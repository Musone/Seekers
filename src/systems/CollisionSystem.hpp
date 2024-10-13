#pragma once

#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include <vector>
#include <glm/geometric.hpp>

#include "utils/Log.hpp"

namespace CollisionSystem {
    inline void check_collisions()
    {
        Registry& registry = Registry::get_instance();

        for (unsigned int i = 0; i < registry.bounding_boxes.entities.size(); i++) {
            Entity& entity_i = registry.bounding_boxes.entities[i];
            Motion& motion_i = registry.motions.get(entity_i);
            BoundingBox& box_i = registry.bounding_boxes.get(entity_i);

            for(unsigned int j = i+1; j < registry.bounding_boxes.entities.size(); j++) {
                Entity& entity_j = registry.bounding_boxes.entities[j];
                Motion& motion_j = registry.motions.get(entity_j);
                BoundingBox& box_j = registry.bounding_boxes.get(entity_j);

                float distance = glm::length(motion_j.position - motion_i.position);
                float combined_radius = box_j.radius + box_i.radius;
                if (distance < combined_radius)
                {
                    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                }
            }
        }
    }

    inline void handle_collisions()
    {
        Registry& registry = Registry::get_instance();
        for (unsigned int i = 0; i < registry.collisions.entities.size(); i++) {
            Entity& entity1 = registry.collisions.entities[i];
            Entity& entity2 = registry.collisions.get(entity1).other;

            if (registry.teams.get(entity1).team_id != registry.teams.get(entity2).team_id) {
                if (registry.projectile_stats.has(entity1) && registry.locomotion_stats.has(entity2)) {
                    registry.locomotion_stats.get(entity2).health -= registry.projectile_stats.get(entity1).damage;
                    registry.remove_all_components_of(entity1);
                    Log::log_info("Projectile hit target.", __FILE__, __LINE__);
                    if (registry.locomotion_stats.get(entity2).health <= 0) {
                        unsigned int weapon_id = registry.attackers.get(entity2).weapon_id;
                        registry.move_withs.remove(weapon_id);
                        registry.rotate_withs.remove(weapon_id);
                        registry.remove_all_components_of(entity2);
                    }
                } else if (registry.projectile_stats.has(entity2) && registry.locomotion_stats.has(entity1)) {
                    registry.locomotion_stats.get(entity1).health -= registry.projectile_stats.get(entity2).damage;
                    registry.remove_all_components_of(entity2);
                    Log::log_info("Projectile hit target.", __FILE__, __LINE__);
                    if (registry.locomotion_stats.get(entity1).health <= 0) {
                        unsigned int weapon_id = registry.attackers.get(entity1).weapon_id;
                        registry.move_withs.remove(weapon_id);
                        registry.rotate_withs.remove(weapon_id);
                        registry.remove_all_components_of(entity1);
                    }
                } else if (registry.locomotion_stats.has(entity1) && registry.locomotion_stats.has(entity2)) {
                    Log::log_info("Locomotion entities collided.", __FILE__, __LINE__);
                    Motion& motion1 = registry.motions.get(entity1);
                    Motion& motion2 = registry.motions.get(entity2);
                    glm::vec2 delta = motion1.position - motion2.position;
                    float overlap = registry.bounding_boxes.get(entity1).radius + registry.bounding_boxes.get(entity2).radius - glm::length(delta);
                    glm::vec2 direction = glm::normalize(delta);
                    motion1.position += direction * overlap + 0.01f;
                    motion2.position -= direction * overlap + 0.01f;
                }
            }
        }
        registry.collisions.clear();
    }
};
