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

                if (registry.teams.get(entity_i).team_id != registry.teams.get(entity_j).team_id) {
                    float distance = glm::length(motion_j.position - motion_i.position);
                    float combined_radius = box_j.radius + box_i.radius;
                    if (distance < combined_radius)
                    {
                        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                    }
                }
            }
        }
    }

    inline void proj_loco_collision(Entity& proj, Entity& loco) {
        Registry& registry = Registry::get_instance();

        registry.locomotion_stats.get(loco).health -= registry.projectile_stats.get(proj).damage;
        registry.remove_all_components_of(proj);
        Log::log_info("Projectile hit target.", __FILE__, __LINE__);
        if (registry.locomotion_stats.get(loco).health <= 0) {
            unsigned int weapon_id = registry.attackers.get(loco).weapon_id;
            registry.move_withs.remove(weapon_id);
            registry.rotate_withs.remove(weapon_id);
            registry.remove_all_components_of(loco);
        }
    }

    inline void loco_loco_collision(Entity& loco1, Entity& loco2) {
        Registry& registry = Registry::get_instance();

        // Log::log_info("Locomotion entities collided.", __FILE__, __LINE__);
        Motion& motion1 = registry.motions.get(loco1);
        Motion& motion2 = registry.motions.get(loco2);
        glm::vec2 delta = motion1.position - motion2.position;
        float overlap = registry.bounding_boxes.get(loco1).radius + registry.bounding_boxes.get(loco2).radius - glm::length(delta);
        glm::vec2 direction = glm::normalize(delta);
        motion1.position += direction * (overlap + 0.04f);
        motion2.position -= direction * (overlap + 0.04f);
    }

    inline void proj_fixed_collision(Entity& proj, Entity& fixed) {
        Registry& registry = Registry::get_instance();

        registry.remove_all_components_of(proj);
        // Log::log_info("Projectile hit a fixed entity.", __FILE__, __LINE__);
    }

    inline void loco_fixed_collision(Entity& loco, Entity& fixed) {
        Registry& registry = Registry::get_instance();

        // Log::log_info("Locomotion entity collided with fixed entity.", __FILE__, __LINE__);
        if (!registry.motions.has(loco) || !registry.motions.has(fixed)) {return;}
        Motion& loco_motion = registry.motions.get(loco);
        Motion& fixed_motion = registry.motions.get(fixed);
        auto pushback_dir = Common::normalize(loco_motion.velocity) * -1.0f;
        glm::vec2 delta = loco_motion.position - fixed_motion.position;
        float overlap = registry.bounding_boxes.get(loco).radius + registry.bounding_boxes.get(fixed).radius - glm::length(delta);
        glm::vec2 direction = glm::normalize(delta);
        loco_motion.position += pushback_dir * (overlap + 0.04f);
        delta = loco_motion.position - fixed_motion.position;
        overlap = registry.bounding_boxes.get(loco).radius + registry.bounding_boxes.get(fixed).radius - glm::length(delta);
        if (overlap > 0.0f) {
            loco_motion.position += direction * (overlap + 0.04f);
        }
        if (registry.in_dodges.has(loco)) {registry.in_dodges.remove(loco);}
    }

    inline void handle_collisions()
    {
        Registry& registry = Registry::get_instance();
        for (unsigned int i = 0; i < registry.collisions.entities.size(); i++) {
            Entity& entity1 = registry.collisions.entities[i];
            Entity& entity2 = registry.collisions.get(entity1).other;

            // check if one of the entities were removed in previous collision resolve
            if (!registry.teams.has(entity2) || !registry.teams.has(entity1)) {continue;}

            if (registry.projectile_stats.has(entity1)) {
                if (registry.locomotion_stats.has(entity2)) {
                    proj_loco_collision(entity1, entity2);
                } else {
                    proj_fixed_collision(entity1, entity2);
                }
            } else if (registry.locomotion_stats.has(entity1)) {
                if (registry.projectile_stats.has(entity2)) {
                    proj_loco_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_loco_collision(entity1, entity2);
                } else {
                    loco_fixed_collision(entity1, entity2);
                }
            } else {
                if (registry.projectile_stats.has(entity2)) {
                    proj_fixed_collision(entity2, entity1);
                } else if (registry.locomotion_stats.has(entity2)) {
                    loco_fixed_collision(entity2, entity1);
                }
            }
        }
        registry.collisions.clear();
    }
};
