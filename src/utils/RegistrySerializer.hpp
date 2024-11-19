#pragma once
#include "ComponentSerializer.hpp"
#include "ecs/Registry.hpp"
#include <vector>

class RegistrySerializer {
private:
    using EntityMap = std::vector<std::pair<unsigned int, Entity>>;
    
    static Entity find_mapped_entity(const EntityMap& map, unsigned int old_id) {
        auto it = std::find_if(map.begin(), map.end(),
            [old_id](const auto& pair) { return pair.first == old_id; });
        return it != map.end() ? it->second : Entity();
    }

    static json serialize_entity_components(Registry& registry, Entity entity) {
        json entity_data;
        entity_data["id"] = entity.get_id();
        
        // Motion component
        if (registry.motions.has(entity)) {
            entity_data["motion"] = ComponentSerializer::serialize_motion(
                registry.motions.get(entity));
        }
        
        // TODO: Add serialization for other components:
        // - registry.enemies
        // - registry.walls
        // - registry.static_objects
        // - registry.weapons
        // - registry.attackers
        // - registry.ai_components
        
        return entity_data;
    }

public:
    static json serialize_registry(Registry& registry) {
        json registry_data;
        registry_data["entities"] = json::array();
        
        // Helper to check if entity already serialized
        auto is_entity_serialized = [&registry_data](unsigned int entity_id) {
            for (const auto& entity_json : registry_data["entities"]) {
                if (entity_json["id"] == entity_id) {
                    return true;
                }
            }
            return false;
        };

        // Serialize entities with Motion components
        for (Entity& entity : registry.motions.entities) {
            if (!is_entity_serialized(entity.get_id())) {
                registry_data["entities"].push_back(
                    serialize_entity_components(registry, entity));
            }
        }
        
        // TODO: Add serialization for other component types here

        // Save player entity if it exists
        if (registry.motions.has(registry.player)) {
            registry_data["player_id"] = registry.player.get_id();
        }
        
        return registry_data;
    }
    
    static Entity deserialize_entity(Registry& registry, const json& entity_data) {
        // Create a new entity (ID is auto-assigned)
        Entity entity;
        unsigned int new_id = entity.get_id();
        Log::log_info("Deserialized entity with new ID: " + std::to_string(new_id), __FILE__, __LINE__);
        
        // Deserialize Motion component if present
        if (entity_data.contains("motion")) {
            Motion motion;
            ComponentSerializer::deserialize_motion(motion, entity_data["motion"]);
            registry.motions.emplace(entity) = motion;
            Log::log_info("Deserialized motion component for entity ID: " + std::to_string(new_id), __FILE__, __LINE__);
        }
        
        // TODO: Add deserialization for other components here
        
        return entity;
    }
    
    static void deserialize_registry(Registry& registry, const json& registry_data) {
        if (!registry_data.contains("entities")) {
            throw SerializationError("Missing 'entities' in registry data");
        }
        
        registry.clear_all_components();
        
        // Variable to store the player entity
        Entity loaded_player;

        // First pass: create all entities
        for (const auto& entity_data : registry_data["entities"]) {
            if (!entity_data.contains("id")) {
                throw SerializationError("Entity missing 'id' field");
            }
            // Note: We are ignoring the saved ID since Entity auto-assigns a new one
            Entity new_entity = deserialize_entity(registry, entity_data);
            
            // Identify if this is the player entity
            if (registry_data.contains("player_id") && entity_data["id"] == registry_data["player_id"]) {
                loaded_player = new_entity;
                Log::log_info("Identified player entity with new ID: " + std::to_string(new_entity.get_id()), __FILE__, __LINE__);
            }
        }
        
        // Restore player entity
        if (registry_data.contains("player_id")) {
            if (loaded_player.get_id() != 0) { // Assuming Entity() default ID is 0
                registry.player = loaded_player;
                Log::log_info("Set registry.player to entity ID " + std::to_string(loaded_player.get_id()), __FILE__, __LINE__);
            } else {
                throw SerializationError("Player entity not found in loaded data");
            }
        }
    }
};
