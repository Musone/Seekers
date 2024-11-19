#pragma once
#include "ComponentSerializer.hpp"
#include "ecs/Registry.hpp"
#include <vector>
#include <set>

namespace std {
    template<>
    struct less<Entity> {
        bool operator()(const Entity& lhs, const Entity& rhs) const {
            return lhs.get_id() < rhs.get_id();
        }
    };
}

class RegistrySerializer {
private:
    using EntityMap = std::vector<std::pair<unsigned int, Entity>>;
    
    static Entity find_mapped_entity(const EntityMap& map, unsigned int old_id) {
        auto it = std::find_if(map.begin(), map.end(),
            [old_id](const auto& pair) { return pair.first == old_id; });
        return it != map.end() ? it->second : Entity();
    }

    static bool is_valid_entity(const Entity& entity) {
        return entity.get_id() != 0;  // Assuming 0 is invalid ID
    }

    static json serialize_entity_components(Registry& registry, Entity entity) {
        json entity_data;
        entity_data["id"] = entity.get_id();
        
        if (registry.motions.has(entity)) {
            entity_data["motion"] = ComponentSerializer::serialize_motion(
                registry.motions.get(entity));
        }

        if (registry.locomotion_stats.has(entity)) {
            entity_data["locomotion_stats"] = ComponentSerializer::serialize_locomotion_stats(
                registry.locomotion_stats.get(entity));
        }

        if (registry.weapons.has(entity)) {
            entity_data["weapon"] = ComponentSerializer::serialize_weapon(
                registry.weapons.get(entity));
        }
    
        if (registry.teams.has(entity)) {
            entity_data["team"] = ComponentSerializer::serialize_team(
                registry.teams.get(entity));
        }
        
        if (registry.attackers.has(entity)) {
            const auto& attacker = registry.attackers.get(entity);
            json attacker_data;
            attacker_data["aim"] = Serialization::serialize_vec2(attacker.aim);
            if (is_valid_entity(attacker.weapon)) {
                attacker_data["weapon_id"] = attacker.weapon.get_id();
            }
            entity_data["attacker"] = attacker_data;
        }
        
        if (registry.collision_bounds.has(entity)) {
            entity_data["collision_bounds"] = ComponentSerializer::serialize_collision_bounds(
                registry.collision_bounds.get(entity));
        }

        if (registry.walls.has(entity)) {
            entity_data["wall"] = ComponentSerializer::serialize_wall(
                registry.walls.get(entity));
        }

        if (registry.enemies.has(entity)) {
            entity_data["enemy"] = ComponentSerializer::serialize_enemy(
                registry.enemies.get(entity));
        }

        if (registry.static_objects.has(entity)) {
            entity_data["static_object"] = ComponentSerializer::serialize_static_object(
                registry.static_objects.get(entity));
        }

        if (registry.ais.has(entity)) {
            entity_data["ai"] = ComponentSerializer::serialize_ai_component(
                registry.ais.get(entity));
        }

        if (registry.move_withs.has(entity)) {
            entity_data["move_with"] = ComponentSerializer::serialize_move_with(
                registry.move_withs.get(entity));
        }

        if (registry.rotate_withs.has(entity)) {
            entity_data["rotate_with"] = ComponentSerializer::serialize_rotate_with(
                registry.rotate_withs.get(entity));
        }

        // TODO: Add serialization methods for other components:
        // - [ ] Collisions
        // - [ ] Buffs
        // - [ ] Projectiles
        // - [ ] AttackCooldown
        // - [ ] TextureName
        // - [ ] InDodge
        // - [ ] NearPlayer
        // - [ ] NearCamera
        // - [ ] StaggerCooldown
        // - [ ] DeathCooldown
        // - [ ] EnergyNoRegenCooldown
        // - [ ] VisionToPlayer
        // - [ ] ProjectileModels

        return entity_data;
    }

public:
    static json serialize_registry(Registry& registry) {
        json registry_data;
        std::set<Entity> serialized_entities;
        
        // Collect all unique entities from component containers
        auto collect_entities = [&serialized_entities](const auto& component_container) {
            for (const auto& entity : component_container.entities) {
                serialized_entities.insert(entity);
            }
        };
        
        // Collect from all component containers
        collect_entities(registry.motions);
        collect_entities(registry.locomotion_stats);
        collect_entities(registry.weapons);
        collect_entities(registry.teams);
        collect_entities(registry.attackers);
        collect_entities(registry.collision_bounds);
        collect_entities(registry.walls);
        collect_entities(registry.enemies);
        collect_entities(registry.static_objects);
        collect_entities(registry.ais);
        collect_entities(registry.move_withs);
        collect_entities(registry.rotate_withs);

        // TODO: Collect other component containers:
        // - [ ] Collisions
        // - [ ] Buffs
        // - [ ] Projectiles
        // - [ ] AttackCooldown
        // - [ ] TextureName
        // - [ ] InDodge
        // - [ ] NearPlayer
        // - [ ] NearCamera
        // - [ ] StaggerCooldown
        // - [ ] DeathCooldown
        // - [ ] EnergyNoRegenCooldown
        // - [ ] VisionToPlayer
        // - [ ] ProjectileModels
        
        // Store global registry state
        registry_data["counter"] = registry.counter;
        registry_data["camera_pos"] = Serialization::serialize_vec2(registry.camera_pos);
        
        if (registry.player) {
            registry_data["player_id"] = registry.player.get_id();
        }
        
        // Serialize all collected entities
        registry_data["entities"] = json::array();
        for (Entity entity : serialized_entities) {
            registry_data["entities"].push_back(serialize_entity_components(registry, entity));
        }
        
        return registry_data;
    }
    
    static void deserialize_registry(Registry& registry, const json& registry_data) {
        if (!registry_data.contains("entities")) {
            throw SerializationError("Missing 'entities' in registry data");
        }
        
        registry.clear_all_components();
        EntityMap entity_map;
        
        if (registry_data.contains("counter")) {
            registry.counter = registry_data["counter"];
        }
        if (registry_data.contains("camera_pos")) {
            Serialization::deserialize_vec2(registry.camera_pos, registry_data["camera_pos"]);
        }
        
        // First pass: Create entities and deserialize independent components
        for (const auto& entity_data : registry_data["entities"]) {
            if (!entity_data.contains("id")) {
                throw SerializationError("Entity missing 'id' field");
            }
            
            unsigned int old_id = entity_data["id"];
            Entity new_entity = Entity();
            entity_map.push_back({old_id, new_entity});
            
            if (registry_data.contains("player_id") && old_id == registry_data["player_id"]) {
                registry.player = new_entity;
            }
            
            if (entity_data.contains("motion")) {
                auto& motion = registry.motions.emplace(new_entity);
                ComponentSerializer::deserialize_motion(motion, entity_data["motion"]);
            }
            
            if (entity_data.contains("locomotion_stats")) {
                auto& stats = registry.locomotion_stats.emplace(new_entity);
                ComponentSerializer::deserialize_locomotion_stats(stats, entity_data["locomotion_stats"]);
            }
            
            if (entity_data.contains("weapon")) {
                auto& weapon = registry.weapons.emplace(new_entity);
                ComponentSerializer::deserialize_weapon(weapon, entity_data["weapon"]);
            }
            
            if (entity_data.contains("team")) {
                auto& team = registry.teams.emplace(new_entity);
                ComponentSerializer::deserialize_team(team, entity_data["team"]);
            }
            
            if (entity_data.contains("collision_bounds")) {
                auto& bounds = registry.collision_bounds.emplace(new_entity);
                ComponentSerializer::deserialize_collision_bounds(bounds, entity_data["collision_bounds"]);
            }
            
            if (entity_data.contains("wall")) {
                auto& wall = registry.walls.emplace(new_entity);
                ComponentSerializer::deserialize_wall(wall, entity_data["wall"]);
            }
            
            if (entity_data.contains("enemy")) {
                auto& enemy = registry.enemies.emplace(new_entity);
                ComponentSerializer::deserialize_enemy(enemy, entity_data["enemy"]);
            }
            
            if (entity_data.contains("static_object")) {
                auto& static_obj = registry.static_objects.emplace(new_entity);
                ComponentSerializer::deserialize_static_object(static_obj, entity_data["static_object"]);
            }
            
            if (entity_data.contains("ai")) {
                auto& ai = registry.ais.emplace(new_entity);
                ComponentSerializer::deserialize_ai_component(ai, entity_data["ai"]);
            }
            
            if (entity_data.contains("move_with")) {
                const auto& move_with_data = entity_data["move_with"];
                auto& move_with = registry.move_withs.emplace(new_entity, 
                    move_with_data["following_entity_id"].get<unsigned int>());
                ComponentSerializer::deserialize_move_with(move_with, move_with_data);
            }
            
            if (entity_data.contains("rotate_with")) {
                const auto& rotate_with_data = entity_data["rotate_with"];
                auto& rotate_with = registry.rotate_withs.emplace(new_entity,
                    rotate_with_data["following_entity_id"].get<unsigned int>());
                ComponentSerializer::deserialize_rotate_with(rotate_with, rotate_with_data);
            }

            // TODO: Add deserialization methods for other components:
            // - [ ] Collisions
            // - [ ] Buffs
            // - [ ] Projectiles
            // - [ ] AttackCooldown
            // - [ ] TextureName
            // - [ ] InDodge
            // - [ ] NearPlayer
            // - [ ] NearCamera
            // - [ ] StaggerCooldown
            // - [ ] DeathCooldown
            // - [ ] EnergyNoRegenCooldown
            // - [ ] VisionToPlayer
            // - [ ] ProjectileModels
            
        }
        
        // Second pass: Deserialize components with entity references
        for (const auto& entity_data : registry_data["entities"]) {
            Entity current_entity = find_mapped_entity(entity_map, entity_data["id"]);
            
            if (entity_data.contains("attacker")) {
                auto& attacker = registry.attackers.emplace(current_entity);
                const auto& attacker_data = entity_data["attacker"];
                Serialization::deserialize_vec2(attacker.aim, attacker_data["aim"]);
                
                if (attacker_data.contains("weapon_id")) {
                    Entity weapon_entity = find_mapped_entity(entity_map, attacker_data["weapon_id"]);
                    if (is_valid_entity(weapon_entity)) {
                        attacker.weapon = weapon_entity;
                    }
                }
            }
        }
        
        if (registry_data.contains("player_id") && !registry.player) {
            throw SerializationError("Player entity specified but not found in entities");
        }
    }
};
