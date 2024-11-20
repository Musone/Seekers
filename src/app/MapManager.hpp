#pragma once

#include "ecs/Registry.hpp"
#include "systems/ProceduralGenerationSystem.hpp"
#include "systems/OpenWorldMapCreatorSystem.hpp"

class MapManager {
public:
    static MapManager& get_instance() {
        static MapManager instance;
        return instance;
    }

    // Called once when a new game is started. Load game should not call this.
    void initialize_maps() {
        if (!open_world_registry) {
            open_world_registry = std::make_unique<Registry>();
            Registry& registry = *open_world_registry;

            auto player = EntityFactory::create_player(registry, glm::vec2(0.0f, 0.0f));
            auto weapon = EntityFactory::create_weapon(registry, glm::vec2(10.0f, 5.0f), 10.0f);
            registry.attackers.get(player).weapon = weapon;
            while (registry.inventory.estus.size() < 3) {
                Entity e = Entity();
                registry.inventory.estus.push_back(e);
                auto& estus = registry.estus.emplace(e);
                estus.heal_amount = 120.0f;
            }

            // add dungeon entrance and bonfire here
            EntityFactory::create_bonfire(registry, glm::vec2(10.0f, 10.0f));
            EntityFactory::create_portal(registry, glm::vec2(-10.0f, -10.0f), INTERACTABLE_TYPE::DUNGEON_ENTRANCE);

            EntityFactory::create_light_source(registry, {0, 0, 100}, 150, {1, 1, 0.8}, LIGHT_SOURCE_TYPE::SUN);

            OpenWorldMapCreatorSystem::populate_open_world_map(registry);

            saved_world_registry = std::make_unique<Registry>();
            *saved_world_registry = *open_world_registry;

            set_theme("OpenWorld");
        }
        // if (!spire_one_registry) {
        //     spire_one_registry = std::make_unique<Registry>();
        //     // Populate spire1 entities here (player should not be added, just the level)
        // }
        // if (!spire_two_registry) {
        //     spire_two_registry = std::make_unique<Registry>();
        //     // Populate spire2 entities here (player should not be added, just the level)
        // }
        // if (!spire_three_registry) {
        //     spire_three_registry = std::make_unique<Registry>();
        //     // Populate spire3 entities here (player should not be added, just the level)
        // }
        active_registry = open_world_registry.get();
    }

    // Called on respawns (ie. player death)
    void restart_maps() {
        assert(saved_world_registry && "saved_world_registry was not initialized but respawn is triggered.");

        dungeon_registry.reset();
        open_world_registry.reset();
        open_world_registry = std::make_unique<Registry>();
        *open_world_registry = *saved_world_registry;
        active_registry = open_world_registry.get();
        set_theme("OpenWorld");
        Globals::restart_renderer = true;
    }

    void load_maps() {
        // TODO: Ahmad load the registries here and we'll call this function when loading a saved game
        // game can only be saved in open world so there is no need to create and load dungeon registry
        if (!open_world_registry) {
            open_world_registry = std::make_unique<Registry>();
            // Populate open world entities here
        }
        active_registry = open_world_registry.get();
    }

    // checks the flags and switches the maps if necessary
    void switch_map() {
        if (return_open_world_flag) {
            return_open_world_flag = false;
            if (active_registry == open_world_registry.get()) {
                std::cout << "Already in open world. Return operation is illegal." << std::endl;
                return;
            }
            return_to_world();
        } else if (enter_dungeon_flag) {
            enter_dungeon_flag = false;
            if (active_registry == dungeon_registry.get()) {
                std::cout << "Already in dungeon. Enter operation is illegal." << std::endl;
                return;
            }
            enter_dungeon();
        }
    }

    Registry& get_active_registry() const {
        return *active_registry;
    }

    bool return_open_world_flag = false;
    bool enter_dungeon_flag = false;
    // bool enter_spire_one_flag = false;
    // bool enter_spire_two_flag = false;
    // bool enter_spire_three_flag = false;

    std::string sky_texture_name;
    std::string wall_texture_name;
    std::string floor_texture_name;


private:
    MapManager() = default;
    MapManager(const MapManager&) = delete;
    void operator=(const MapManager&) = delete;

    void set_theme(std::string theme) {
        if (theme == "OpenWorld") {
            sky_texture_name = "Blue sky.png";
            wall_texture_name = "jungle_tile_1.jpg";
            floor_texture_name = "ground.jpg";
        } else if (theme == "Dungeon") {
            sky_texture_name = "random_skybox.png";
            wall_texture_name = "jungle_tile_1.jpg";
            floor_texture_name = "jungle_tile_1.jpg";
        }
    }

    void enter_dungeon() {
        dungeon_registry = std::make_unique<Registry>();
        active_registry = dungeon_registry.get();
        move_player_comps(*open_world_registry, *dungeon_registry);
        ProceduralGenerationSystem::generate_dungeon(*dungeon_registry, MAP_WIDTH, MAP_HEIGHT, dungeon_registry->motions.get(dungeon_registry->player));
        // dungeon_registry->projectile_models = open_world_registry->projectile_models;
        set_theme("Dungeon");
        Globals::restart_renderer = true;
    }

    void return_to_world() {
        active_registry = open_world_registry.get();
        Motion player_motion_copy = open_world_registry->motions.get(open_world_registry->player);
        move_player_comps(*dungeon_registry, *open_world_registry);
        open_world_registry->motions.get(open_world_registry->player) = player_motion_copy;
        dungeon_registry.reset();
        set_theme("OpenWorld");
        Globals::restart_renderer = true;
    }

    void move_player_comps(Registry& from, Registry& to) {
        to.player = from.player;

        if (to.attackers.has(to.player)) {  // in case weapon is dropped in dungeon, we don't want to keep it in memory.
            to.remove_all_components_of(to.attackers.get(to.player).weapon);
        }
        to.remove_all_components_of(from.player);

        auto& motion_from = from.motions.get(from.player);
        auto& motion_to = to.motions.emplace(to.player);
        motion_to = motion_from;

        auto& loco_from = from.locomotion_stats.get(from.player);
        auto& loco_to = to.locomotion_stats.emplace(to.player);
        loco_to = loco_from;

        auto& team_from = from.teams.get(from.player);
        auto& team_to = to.teams.emplace(to.player);
        team_to = team_from;

        auto& attacker_from = from.attackers.get(from.player);
        auto& attacker_to = to.attackers.emplace(to.player);
        attacker_to = attacker_from;
        move_player_weapon(from, to, attacker_from.weapon);

        auto& collision_from = from.collision_bounds.get(from.player);
        auto& collision_to = to.collision_bounds.emplace(to.player);
        collision_to = collision_from;

        to.inventory = from.inventory;
        to.estus = from.estus;
    }

    void move_player_weapon(Registry& from, Registry& to, Entity weapon) {
        to.remove_all_components_of(weapon);

        auto& motion_from = from.motions.get(weapon);
        auto& motion_to = to.motions.emplace(weapon);
        motion_to = motion_from;

        auto& weapon_from = from.weapons.get(weapon);
        auto& weapon_to = to.weapons.emplace(weapon);
        weapon_to = weapon_from;
    }

    std::unique_ptr<Registry> open_world_registry;    // Persistent open world registry
    std::unique_ptr<Registry> dungeon_registry;       // Temporary dungeon registry
    // std::unique_ptr<Registry> spire_one_registry;     // Spire1 registry for future use
    // std::unique_ptr<Registry> spire_two_registry;     // Spire2 registry for future use
    // std::unique_ptr<Registry> spire_three_registry;   // Spire3 registry for future use
    std::unique_ptr<Registry> saved_world_registry;   // Instance of last saved checkpoint (only open_world has save ability)
    Registry* active_registry = nullptr;              // Points to the currently active registry
};
