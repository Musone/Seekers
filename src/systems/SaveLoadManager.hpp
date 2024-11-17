#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <ctime>
#include <boost/filesystem.hpp>
#include "app/MapManager.hpp"
#include "components/Components.hpp"

using json = nlohmann::json;
namespace fs = boost::filesystem;

class SaveLoadManager {
public:
    static SaveLoadManager& get_instance() {
        static SaveLoadManager instance;
        return instance;
    }

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
            
            fs::create_directories("saves");
            std::ofstream file("saves/" + save_name + ".json");
            file << std::setw(4) << save_data << std::endl;
            
            std::cout << "Game saved successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Save failed: " << e.what() << std::endl;
            return false;
        }
    }

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

private:
    SaveLoadManager() = default;

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
}; 