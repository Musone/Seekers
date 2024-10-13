#include "CollisionSystem.hpp"
#include "../utils/Log.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

CollisionSystem::CollisionSystem() : m_registry(Registry::get_instance()) {}

void CollisionSystem::check_collisions()
{
    ComponentContainer<Motion> &motions = m_registry.motions;
    for (unsigned int i = 0; i < motions.components.size(); i++)
    {
        Motion& motion_i = motions.components[i];
        Entity entity_i = motions.entities[i];
        
        // Compare with entities after i to avoid duplicate checks
        for(unsigned int j = i+1; j < motions.components.size(); j++)
        {
            Motion& motion_j = motions.components[j];
            if (check_collision(motion_i, motion_j))
            {
                Entity entity_j = motions.entities[j];
                create_collision(entity_i, entity_j);
            }
        }
    }
}

bool CollisionSystem::check_collision(const Motion& motion1, const Motion& motion2) const
{
    // Implement circular collision detection
    float distance = glm::length(motion1.position - motion2.position);
    float combined_radius = motion1.scale.x / 2.0f + motion2.scale.x / 2.0f;

    return distance < combined_radius;
}

void CollisionSystem::create_collision(Entity& entity1, Entity& entity2)
{
    m_registry.collisions.emplace_with_duplicates(entity1, entity2);
    m_registry.collisions.emplace_with_duplicates(entity2, entity1);
}

void CollisionSystem::get_nearby_entities(const Entity& entity) const
{
    // TODO: Implement spatial partitioning for better performance
    // For now, return all entities with Motion component
}
