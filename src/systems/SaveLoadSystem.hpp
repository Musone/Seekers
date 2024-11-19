#pragma once
#include "ecs/Registry.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <unordered_set>
#include "app/MapManager.hpp"

using json = nlohmann::json;

struct SaveSlot {
    int id;                     // Unique incremental ID
    std::string name;           // User-given name
    std::string filename;       // Actual file name on disk
    std::time_t timestamp;      // When save was created
    std::string version;        // Game version
};

class SaveLoadSystem {
public:
    static SaveLoadSystem& get_instance() {
        static SaveLoadSystem instance;
        return instance;
    }

    std::vector<SaveSlot> list_save_slots() {
        return save_slots;
    }

    SaveSlot create_new_slot(const std::string& name) {
        SaveSlot slot;
        slot.id = next_slot_id++;
        slot.name = name;
        slot.filename = "save_" + std::to_string(slot.id) + ".json";
        slot.timestamp = std::time(nullptr);
        slot.version = "1.0.0";
        save_slots.push_back(slot);
        save_index_file();
        return slot;
    }

    bool save_game_to_slot(const SaveSlot& slot) {
        try {
            auto& map_manager = MapManager::get_instance();
            if (map_manager.get_current_map_type() != MapType::OPEN_WORLD) {
                Log::log_warning("Can only save in open world (SaveLoadSystem::save_game_to_slot)", __FILE__, __LINE__);
                return false;
            }

            json save_data;
            save_data["version"] = slot.version;
            save_data["timestamp"] = slot.timestamp;
            
            auto& registry = map_manager.get_active_registry();
            save_data["entities"] = serialize_entities(registry);
            
            #ifdef _WIN32
                _mkdir("saves");
            #else
                mkdir("saves", 0777);
            #endif
            
            std::ofstream file("saves/" + slot.filename);
            file << std::setw(4) << save_data << std::endl;
            return true;
        } catch (const std::exception& e) {
            Log::log_warning("Save failed (SaveLoadSystem::save_game_to_slot): " + std::string(e.what()), __FILE__, __LINE__);
            return false;
        }
    }

    bool load_game_from_slot(const SaveSlot& slot) {
        try {
            std::ifstream file("saves/" + slot.filename);
            if (!file.is_open()) {
                Log::log_info("Save file not found (SaveLoadSystem::load_game_from_slot): " + slot.filename, __FILE__, __LINE__);
                return false;
            }

            json save_data = json::parse(file);
            
            auto& map_manager = MapManager::get_instance();
            map_manager.restart_maps();  // Reset to initial state
            
            auto& registry = map_manager.get_active_registry();
            std::unordered_map<unsigned int, Entity> entity_map;
            deserialize_entities(registry, save_data["entities"]);
            
            return true;
        } catch (const std::exception& e) {
            Log::log_warning("Load failed (SaveLoadSystem::load_game_from_slot): " + std::string(e.what()), __FILE__, __LINE__);
            return false;
        }
    }

    // Static methods for direct access
    static bool save_game(Registry& registry, const std::string& save_name="untitled") {
        auto& instance = get_instance();
        SaveSlot slot = instance.create_new_slot(save_name);
        return instance.save_game_to_slot(slot);
    }

    static bool load_latest_game(Registry& registry) {
        const auto& slots = get_instance().list_save_slots();
        if (slots.empty()) {
            Log::log_info("No save slots found (SaveLoadSystem::load_latest_game)", __FILE__, __LINE__);
            return false;
        }

        // Find slot with latest timestamp
        const SaveSlot* latest_slot = &slots[0];
        for (const auto& slot : slots) {
            if (slot.timestamp > latest_slot->timestamp) {
                latest_slot = &slot;
            }
        }
        
        return load_game(registry, latest_slot->name);
    }

    static bool load_game(Registry& registry, const std::string& save_name="") {
        if (save_name.empty()) {
            return load_latest_game(registry);
        }

        // Find existing slot with matching name
        const auto& slots = get_instance().list_save_slots();
        for (const auto& slot : slots) {
            if (slot.name == save_name) {
                return get_instance().load_game_from_slot(slot);
            }
        }

        Log::log_info("No save slot found with name: " + save_name, __FILE__, __LINE__);
        return false;
    }

private:
    SaveLoadSystem() {
        load_index_file();
    }

    std::vector<SaveSlot> save_slots;
    int next_slot_id = 0;

    void save_index_file() {
        try {
        #ifdef _WIN32
            _mkdir("saves");
        #else
            mkdir("saves", 0777);
        #endif

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
            Log::log_warning("Failed to save index file (SaveLoadSystem::save_index_file): " + std::string(e.what()), __FILE__, __LINE__);
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
            Log::log_warning("Failed to load index file (SaveLoadSystem::load_index_file): " + std::string(e.what()), __FILE__, __LINE__);
            next_slot_id = 0;
            save_slots.clear();
        }
    }

    static json serialize_input_state(const InputState& input) {
        return {
            {"w_down", input.w_down},
            {"a_down", input.a_down},
            {"s_down", input.s_down},
            {"d_down", input.d_down},
            {"mouse_pos", {input.mouse_pos.x, input.mouse_pos.y}}
        };
    }

    static void deserialize_input_state(InputState& input, const json& data) {
        input.w_down = data["w_down"];
        input.a_down = data["a_down"];
        input.s_down = data["s_down"];
        input.d_down = data["d_down"];
        input.mouse_pos = {
            data["mouse_pos"][0].get<float>(),
            data["mouse_pos"][1].get<float>()
        };
    }

    json serialize_entities(Registry& registry) {
        json entities = json::array();
        std::unordered_map<unsigned int, bool> processed_entities;

        // Start with player
        if (registry.player) {
            processed_entities[registry.player.get_id()] = true;
            entities.push_back(serialize_entity(registry, registry.player));
        }

        // Process other entities by iterating through component containers
        // Using motions container since all entities should have a motion component
        for (auto it = registry.motions.components.begin(); it != registry.motions.components.end(); ++it) {
            unsigned int entity_id = it - registry.motions.components.begin();
            if (!processed_entities[entity_id]) {
                Entity entity;
                processed_entities[entity_id] = true;
                entities.push_back(serialize_entity(registry, entity));
            }
        }

        return entities;
    }

    json serialize_entity(Registry& registry, Entity entity) {
        json entity_data;
        entity_data["id"] = entity.get_id();

        // Serialize each component type if present
        if (registry.motions.has(entity)) {
            const auto& motion = registry.motions.get(entity);
            entity_data["motion"] = {
                {"position", {motion.position.x, motion.position.y}},
                {"angle", motion.angle},
                {"scale", {motion.scale.x, motion.scale.y}},
                {"velocity", {motion.velocity.x, motion.velocity.y}},
                {"acceleration", {motion.acceleration.x, motion.acceleration.y}},
                {"rotation_velocity", motion.rotation_velocity},
                {"drag", motion.drag}
            };
        }

        if (registry.locomotion_stats.has(entity)) {
            const auto& stats = registry.locomotion_stats.get(entity);
            entity_data["locomotion_stats"] = {
                {"health", stats.health},
                {"max_health", stats.max_health},
                {"energy", stats.energy},
                {"max_energy", stats.max_energy},
                {"poise", stats.poise},
                {"max_poise", stats.max_poise},
                {"defense", stats.defense},
                {"power", stats.power},
                {"agility", stats.agility},
                {"movement_speed", stats.movement_speed}
            };
        }

        // Add other component serializations...
        return entity_data;
    }

    void deserialize_entities(Registry& registry, const json& entities) {
        std::unordered_map<unsigned int, Entity> entity_map;
        
        // First pass: create all entities
        for (const auto& entity_data : entities) {
            unsigned int old_id = entity_data["id"];
            Entity new_entity;  // This will create a new entity with a fresh ID
            entity_map[old_id] = new_entity;
        }

        // Second pass: deserialize components
        for (const auto& entity_data : entities) {
            unsigned int old_id = entity_data["id"];
            Entity entity = entity_map[old_id];
            deserialize_entity_components(registry, entity, entity_data, entity_map);
        }
    }

    static void deserialize_entity_components(Registry& registry, Entity entity, 
                                            const json& data,
                                            std::unordered_map<unsigned int, Entity>& entity_map) {
        if (data.contains("motion")) {
            const auto& motion_data = data["motion"];
            auto& motion = registry.motions.emplace(entity);
            motion.position = {motion_data["position"][0], motion_data["position"][1]};
            motion.angle = motion_data["angle"];
            motion.scale = {motion_data["scale"][0], motion_data["scale"][1]};
            motion.velocity = {motion_data["velocity"][0], motion_data["velocity"][1]};
            motion.acceleration = {motion_data["acceleration"][0], motion_data["acceleration"][1]};
            motion.rotation_velocity = motion_data["rotation_velocity"];
            motion.drag = motion_data["drag"];
        }

        if (data.contains("locomotion_stats")) {
            const auto& stats_data = data["locomotion_stats"];
            auto& stats = registry.locomotion_stats.emplace(entity);
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

        // Add other component deserializations following the same pattern...
    }
};
