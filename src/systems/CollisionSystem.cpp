#include "CollisionSystem.hpp"
#include "../utils/Log.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

CollisionSystem::CollisionSystem() : m_registry(Registry::get_instance()) {}

void CollisionSystem::check_collisions()
{
    auto& motions = m_registry.motions;
    for (size_t i = 0; i < motions.entities.size(); ++i)
    {
        Entity entity1 = motions.entities[i];
        auto nearby_entities = get_nearby_entities(entity1);
        for (auto entity2 : nearby_entities)
        {
            if (entity1 == entity2) continue;

            if (check_collision(entity1, entity2))
            {
                create_collision(entity1, entity2);
            }
        }
    }
}

bool CollisionSystem::check_collision(const Entity& entity1, const Entity& entity2) const
{
    const auto& motion1 = m_registry.motions.get(entity1);
    const auto& motion2 = m_registry.motions.get(entity2);

    // Implement circular collision detection
    float distance = glm::length(motion1.position - motion2.position);
    float combined_radius = motion1.scale.x / 2.0f + motion2.scale.x / 2.0f;

    return distance < combined_radius;
}

void CollisionSystem::create_collision(const Entity& entity1, const Entity& entity2)
{
    m_registry.collisions.emplace(entity1, entity2);
    m_registry.collisions.emplace(entity2, entity1);
}

std::vector<Entity> CollisionSystem::get_nearby_entities(const Entity& entity) const
{
    // TODO: Implement spatial partitioning for better performance
    // For now, return all entities with Motion component
    std::vector<Entity> nearby_entities;
    auto& motions = m_registry.motions;
    for (size_t i = 0; i < motions.entities.size(); ++i)
    {
        Entity e = motions.entities[i];
        if (e != entity)
        {
            nearby_entities.push_back(e);
        }
    }
    return nearby_entities;
}
