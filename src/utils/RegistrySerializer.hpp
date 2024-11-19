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

        // LocomotionStats component
        if (registry.locomotion_stats.has(entity)) {
            entity_data["locomotion_stats"] = ComponentSerializer::serialize_locomotion_stats(
                registry.locomotion_stats.get(entity));
        }
        
        // TODO: Add serialization for other components:
        // - [X] registry.motions
        // - [X] registry.locomotion_stats
        // - [ ] registry.weapons
        // - [ ] registry.teams
        // - [ ] registry.walls
        // - [ ] registry.enemies
        // - [ ] registry.static_objects
        // - [ ] registry.ai_components
        // - [ ] registry.attackers
        // - [ ] registry.texture_names
        
        return entity_data;
    }

public:
    static json serialize_registry(Registry& registry) {
        json registry_data;
        registry_data["entities"] = json::array();
        registry_data["counter"] = registry.counter;
        registry_data["camera_pos"] = Serialization::serialize_vec2(registry.camera_pos);
        
        // Serialize player ID if it exists
        if (registry.player) {
            registry_data["player_id"] = registry.player.get_id();
        }
        
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

        // Serialize entities with LocomotionStats components
        for (Entity& entity : registry.locomotion_stats.entities) {
            if (!is_entity_serialized(entity.get_id())) {
                registry_data["entities"].push_back(serialize_entity_components(registry, entity));
            }
        }
        
        // TODO: Add serialization for other component types here
        // - [X] registry.motions.entities
        // - [X] registry.locomotion_stats.entities
        // - [ ] registry.weapons.entities
        // - [ ] registry.teams.entities
        // - [ ] registry.walls.entities
        // - [ ] registry.enemies.entities
        // - [ ] registry.static_objects.entities
        // - [ ] registry.ai_components.entities
        // - [ ] registry.attackers.entities
        // - [ ] registry.texture_names.entities

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

        // Deserialize LocomotionStats component if present
        if (entity_data.contains("locomotion_stats")) {
            LocomotionStats stats;
            ComponentSerializer::deserialize_locomotion_stats(stats, entity_data["locomotion_stats"]);
            registry.locomotion_stats.emplace(entity) = stats;
            Log::log_info("Deserialized locomotion stats component for entity ID: " + std::to_string(new_id), __FILE__, __LINE__);
        }
        
        // TODO: Add deserialization for other components here
        // - [X] registry.motions
        // - [X] registry.locomotion_stats
        // - [ ] registry.weapons
        // - [ ] registry.teams
        // - [ ] registry.walls
        // - [ ] registry.enemies
        // - [ ] registry.static_objects
        // - [ ] registry.ai_components
        // - [ ] registry.attackers
        // - [ ] registry.texture_names
        
        return entity;
    }
    
    static void deserialize_registry(Registry& registry, const json& registry_data) {
        if (!registry_data.contains("entities")) {
            throw SerializationError("Missing 'entities' in registry data");
        }
        
        registry.clear_all_components();
        
        // Restore global registry state
        if (registry_data.contains("counter")) {
            registry.counter = registry_data["counter"];
        }
        if (registry_data.contains("camera_pos")) {
            Serialization::deserialize_vec2(registry.camera_pos, registry_data["camera_pos"]);
        }
        
        // First create the player entity if it exists in save
        if (registry_data.contains("player_id")) {
            unsigned int player_id = registry_data["player_id"];
            // Find the player data
            bool found_player = false;
            for (const auto& entity_data : registry_data["entities"]) {
                if (entity_data["id"] == player_id) {
                    Entity player_entity = Entity();
                    registry.player = player_entity;
                    
                    // Deserialize player components
                    if (entity_data.contains("motion")) {
                        auto& motion = registry.motions.emplace(player_entity);
                        ComponentSerializer::deserialize_motion(motion, entity_data["motion"]);
                    }
                    
                    if (entity_data.contains("locomotion_stats")) {
                        auto& stats = registry.locomotion_stats.emplace(player_entity);
                        ComponentSerializer::deserialize_locomotion_stats(stats, entity_data["locomotion_stats"]);
                    }
                    
                    Log::log_info("Created player entity with ID: " + std::to_string(player_entity.get_id()), __FILE__, __LINE__);
                    found_player = true;
                    break;
                }
            }
            
            if (!found_player) {
                throw SerializationError("Player data not found in save file");
            }
        }
        
        // Then create all other entities
        for (const auto& entity_data : registry_data["entities"]) {
            if (!entity_data.contains("id")) {
                throw SerializationError("Entity missing 'id' field");
            }
            
            // Skip if this was the player entity we already created
            if (registry_data.contains("player_id") && 
                entity_data["id"] == registry_data["player_id"]) {
                continue;
            }
            
            Entity new_entity = deserialize_entity(registry, entity_data);
            Log::log_info("Created non-player entity with ID: " + std::to_string(new_entity.get_id()), __FILE__, __LINE__);
        }
    }
};
