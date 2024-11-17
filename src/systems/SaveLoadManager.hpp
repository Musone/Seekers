#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <sys/stat.h>
#include <system_error>
#include "app/MapManager.hpp"
#include "components/Components.hpp"

using json = nlohmann::json;

struct SaveSlot {
    int id;                     // Unique incremental ID
    std::string name;           // User-given name
    std::string filename;       // Actual file name on disk
    std::time_t timestamp;      // When save was created
    std::string version;        // Game version
};

class SaveLoadManager {
public:
    static SaveLoadManager& get_instance() {
        static SaveLoadManager instance;
        return instance;
    }

    // Leaving this in for now until we implement the rest of the save/load system
    bool save_game(const std::string& save_name) {
        try {
            auto& map_manager = MapManager::get_instance();
            if (map_manager.get_current_map_type() != MapType::OPEN_WORLD) {
                std::cerr << "Can only save in open world" << std::endl;
                return false;
            }

            json save_data;
            save_data["version"] = "0.1.0";
            save_data["timestamp"] = std::time(nullptr);
            
            auto& registry = map_manager.get_active_registry();
            save_data["player"] = serialize_player_state(registry);
            
            #ifdef _WIN32
                _mkdir("saves");
            #else
                mkdir("saves", 0777);
            #endif
            
            std::ofstream file("saves/" + save_name + ".json");
            file << std::setw(4) << save_data << std::endl;
            
            std::cout << "Game saved successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Save failed: " << e.what() << std::endl;
            return false;
        }
    }

    // Leaving this in for now until we implement the rest of the save/load system
    bool load_game(const std::string& save_name) {
        try {
            std::ifstream file("saves/" + save_name + ".json");
            if (!file.is_open()) {
                std::cerr << "Save file not found: " << save_name << std::endl;
                return false;
            }

            json save_data = json::parse(file);
            
            auto& map_manager = MapManager::get_instance();
            map_manager.restart_maps();  // Reset to initial state
            
            auto& registry = map_manager.get_active_registry();
            deserialize_player_state(registry, save_data["player"]);
            
            std::cout << "Game loaded successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Load failed: " << e.what() << std::endl;
            return false;
        }
    }

    // Lists all available save slots
    std::vector<SaveSlot> list_save_slots() {
        return save_slots;
    }

    // Creates a new game save slot
    bool create_new_game(const std::string& save_name) {
        // TODO: Create new save slot
        // TODO: Initialize fresh game state
        // TODO: Save to slot
        // TODO: Update index.json
        return false;
    }

    // Save to specific slot
    bool save_game_to_slot(const SaveSlot& slot) {
        try {
            auto& map_manager = MapManager::get_instance();
            if (map_manager.get_current_map_type() != MapType::OPEN_WORLD) {
                std::cerr << "Can only save in open world" << std::endl;
                return false;
            }

            json save_data;
            save_data["version"] = slot.version;
            save_data["timestamp"] = slot.timestamp;
            
            auto& registry = map_manager.get_active_registry();
            save_data["registry"] = serialize_registry(registry);
            
            #ifdef _WIN32
                _mkdir("saves");
            #else
                mkdir("saves", 0777);
            #endif
            
            std::ofstream file("saves/" + slot.filename);
            file << std::setw(4) << save_data << std::endl;
            
            std::cout << "Game saved successfully to slot: " << slot.name << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Save failed: " << e.what() << std::endl;
            return false;
        }
    }

    // Load from specific slot
    bool load_game_from_slot(const SaveSlot& slot) {
        try {
            std::ifstream file("saves/" + slot.filename);
            if (!file.is_open()) {
                std::cerr << "Save file not found: " << slot.filename << std::endl;
                return false;
            }

            json save_data = json::parse(file);
            
            auto& map_manager = MapManager::get_instance();
            map_manager.restart_maps();  // Reset to initial state
            
            auto& registry = map_manager.get_active_registry();
            deserialize_registry(registry, save_data["registry"]);
            
            std::cout << "Game loaded successfully from slot: " << slot.name << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Load failed: " << e.what() << std::endl;
            return false;
        }
    }

    // Delete a save slot
    void delete_save_slot(const SaveSlot& slot) {
        // TODO: Remove save file
        // TODO: Update index.json
        // TODO: Handle errors
    }

    // Creates a new save slot
    SaveSlot create_new_slot(const std::string& name) {
        SaveSlot slot;
        slot.id = next_slot_id++;
        slot.name = name;
        slot.filename = "save_" + std::to_string(slot.id) + ".json";
        slot.timestamp = std::time(nullptr);
        slot.version = "0.1.0"; // TODO: Get from game version
        save_slots.push_back(slot); // Add this line to track the new slot
        save_index_file(); // Add this line to persist changes
        return slot;
    }

private:
    SaveLoadManager() {
        load_index_file(); // Load existing slots on startup
    }
    
    void save_index_file() {
        try {
            json index;
            index["next_slot_id"] = next_slot_id;
            
            std::vector<json> slots_data;
            for (const auto& slot : save_slots) {
                slots_data.push_back({
                    {"id", slot.id},
                    {"name", slot.name},
                    {"filename", slot.filename},
                    {"timestamp", slot.timestamp},
                    {"version", slot.version}
                });
            }
            index["slots"] = slots_data;
            
            std::ofstream file("saves/index.json");
            file << std::setw(4) << index << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to save index file: " << e.what() << std::endl;
        }
    }

    void load_index_file() {
        try {
            std::ifstream file("saves/index.json");
            if (!file.is_open()) {
                next_slot_id = 0;
                save_slots.clear();
                return;
            }

            json index = json::parse(file);
            next_slot_id = index["next_slot_id"];
            save_slots.clear();
            
            for (const auto& slot_data : index["slots"]) {
                SaveSlot slot{
                    slot_data["id"],
                    slot_data["name"],
                    slot_data["filename"],
                    slot_data["timestamp"],
                    slot_data["version"]
                };
                save_slots.push_back(slot);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to load index file: " << e.what() << std::endl;
            next_slot_id = 0;
            save_slots.clear();
        }
    }

    std::vector<SaveSlot> save_slots;
    int next_slot_id = 0;

    // Leaving this in for now until we implement the rest of the save/load system
    json serialize_player_state(Registry& registry) {
        json player_data;
        
        if (registry.player) {
            Entity player = registry.player;
            
            if (auto* motion = registry.try_get_component<Motion>(player)) {
                player_data["motion"] = {
                    {"position", {motion->position.x, motion->position.y}},
                    {"angle", motion->angle},
                    {"scale", {motion->scale.x, motion->scale.y}},
                    {"velocity", {motion->velocity.x, motion->velocity.y}},
                    {"rotation_velocity", motion->rotation_velocity},
                    {"acceleration", {motion->acceleration.x, motion->acceleration.y}},
                    {"drag", motion->drag}
                };
            }
            
            if (auto* stats = registry.try_get_component<LocomotionStats>(player)) {
                player_data["stats"] = {
                    {"health", stats->health},
                    {"max_health", stats->max_health},
                    {"energy", stats->energy},
                    {"max_energy", stats->max_energy},
                    {"poise", stats->poise},
                    {"max_poise", stats->max_poise},
                    {"defense", stats->defense},
                    {"power", stats->power},
                    {"agility", stats->agility},
                    {"movement_speed", stats->movement_speed}
                };
            }
        }
        
        return player_data;
    }

    // Leaving this in for now until we implement the rest of the save/load system
    void deserialize_player_state(Registry& registry, const json& data) {
        if (!registry.player) return;
        
        Entity player = registry.player;
        
        if (data.contains("motion")) {
            auto& motion = registry.get_or_emplace_component<Motion>(player);
            const auto& motion_data = data["motion"];
            motion.position = {motion_data["position"][0], motion_data["position"][1]};
            motion.angle = motion_data["angle"];
            motion.scale = {motion_data["scale"][0], motion_data["scale"][1]};
            motion.velocity = {motion_data["velocity"][0], motion_data["velocity"][1]};
            motion.rotation_velocity = motion_data["rotation_velocity"];
            motion.acceleration = {motion_data["acceleration"][0], motion_data["acceleration"][1]};
            motion.drag = motion_data["drag"];
        }
        
        if (data.contains("stats")) {
            auto& stats = registry.get_or_emplace_component<LocomotionStats>(player);
            const auto& stats_data = data["stats"];
            stats.health = stats_data["health"];
            stats.max_health = stats_data["max_health"];
            stats.energy = stats_data["energy"];
            stats.max_energy = stats_data["max_energy"];
            stats.poise = stats_data["poise"];
            stats.max_poise = stats_data["max_poise"];
            stats.defense = stats_data["defense"];
            stats.power = stats_data["power"];
            stats.agility = stats_data["agility"];
            stats.movement_speed = stats_data["movement_speed"];
        }
    }

    // Helper functions for serialization
    json serialize_combat_components(Registry& registry) {
        json combat;
        // TODO: Implement all combat component serialization
        // TODO: Serialize Attacker (aim, weapon)
        // TODO: Serialize Weapon (type, damage, range, etc)
        // TODO: Serialize Projectile (damage, range_remaining, etc)
        // TODO: Serialize AttackCooldown (timer)
        // TODO: Serialize StaggerCooldown (timer)
        // TODO: Serialize DeathCooldown (timer)
        // TODO: Serialize EnergyNoRegenCooldown (timer)
        // TODO: Serialize Team (team_id)
        return combat;
    }

    json serialize_gameplay_components(Registry& registry) {
        json gameplay;
        // TODO: Implement all gameplay component serialization
        // TODO: Serialize LocomotionStats (health, energy, etc)
        // TODO: Serialize Buff (remaining_time, effects, etc)
        // TODO: Serialize InDodge (source, destination, etc)
        // TODO: Serialize NearPlayer
        // TODO: Serialize NearCamera
        return gameplay;
    }

    json serialize_physics_components(Registry& registry) {
        json physics;
        // TODO: Implement all physics component serialization
        // TODO: Serialize CollisionBounds (type and specific collider data)
        // TODO: Serialize Collision (other entity)
        // TODO: Serialize MoveWith (following_entity_id)
        // TODO: Serialize RotateWith (following_entity_id)
        return physics;
    }

    json serialize_ai_components(Registry& registry) {
        json ai;
        // TODO: Implement all AI component serialization
        // TODO: Serialize AIComponent (current_state, target_position, etc)
        // TODO: Serialize VisionToPlayer (timer)
        return ai;
    }

    json serialize_identifier_components(Registry& registry) {
        json identifiers;
        // TODO: Implement all identifier component serialization
        // TODO: Serialize Wall (type)
        // TODO: Serialize Enemy (type)
        // TODO: Serialize StaticObject (type)
        return identifiers;
    }

    json serialize_render_components(Registry& registry) {
        json render;
        // TODO: Implement all render component serialization
        // TODO: Serialize TextureName (name)
        // TODO: Serialize ProjectileModels (model pointers - needs special handling)
        return render;
    }

    // Main serialization function becomes cleaner
    json serialize_registry(Registry& registry) {
        json data;
        
        // Registry-level data
        data["player"] = static_cast<unsigned int>(registry.player);
        data["camera_pos"] = {
            {"x", registry.camera_pos.x},
            {"y", registry.camera_pos.y}
        };
        data["input_state"] = {
            {"w_down", registry.input_state.w_down},
            {"a_down", registry.input_state.a_down},
            {"s_down", registry.input_state.s_down},
            {"d_down", registry.input_state.d_down},
            {"mouse_pos", {
                registry.input_state.mouse_pos.x,
                registry.input_state.mouse_pos.y
            }}
        };

        // Components by category
        data["combat"] = serialize_combat_components(registry);
        data["gameplay"] = serialize_gameplay_components(registry);
        data["physics"] = serialize_physics_components(registry);
        data["ai"] = serialize_ai_components(registry);
        data["identifiers"] = serialize_identifier_components(registry);
        data["render"] = serialize_render_components(registry);
        
        return data;
    }

    // Helper functions for deserialization
    void deserialize_combat_components(Registry& registry, const json& data) {
        // TODO: Implement all combat component deserialization
        // TODO: Deserialize Attacker
        // TODO: Deserialize Weapon
        // TODO: Deserialize Projectile
        // TODO: Deserialize AttackCooldown
        // TODO: Deserialize StaggerCooldown
        // TODO: Deserialize DeathCooldown
        // TODO: Deserialize EnergyNoRegenCooldown
        // TODO: Deserialize Team
    }

    void deserialize_gameplay_components(Registry& registry, const json& data) {
        // TODO: Implement all gameplay component deserialization
        // TODO: Deserialize LocomotionStats
        // TODO: Deserialize Buff
        // TODO: Deserialize InDodge
        // TODO: Deserialize NearPlayer
        // TODO: Deserialize NearCamera
    }

    void deserialize_physics_components(Registry& registry, const json& data) {
        // TODO: Implement all physics component deserialization
        // TODO: Deserialize CollisionBounds
        // TODO: Deserialize Collision
        // TODO: Deserialize MoveWith
        // TODO: Deserialize RotateWith
    }

    void deserialize_ai_components(Registry& registry, const json& data) {
        // TODO: Implement all AI component deserialization
        // TODO: Deserialize AIComponent
        // TODO: Deserialize VisionToPlayer
    }

    void deserialize_identifier_components(Registry& registry, const json& data) {
        // TODO: Implement all identifier component deserialization
        // TODO: Deserialize Wall
        // TODO: Deserialize Enemy
        // TODO: Deserialize StaticObject
    }

    void deserialize_render_components(Registry& registry, const json& data) {
        // TODO: Implement all render component deserialization
        // TODO: Deserialize TextureName
        // TODO: Deserialize ProjectileModels
    }

    void deserialize_registry(Registry& registry, const json& data) {
        // TODO: Clear existing registry
        // TODO: Restore registry.player
        // TODO: Restore registry.input_state
        // TODO: Restore registry.camera_pos
        
        // Components by category
        deserialize_combat_components(registry, data);
        deserialize_gameplay_components(registry, data);
        deserialize_physics_components(registry, data);
        deserialize_ai_components(registry, data);
        deserialize_identifier_components(registry, data);
        deserialize_render_components(registry, data);
        
        // For now, use existing player deserialization
        deserialize_player_state(registry, data["player"]);
    }
}; 