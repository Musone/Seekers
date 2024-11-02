#include "World.h"
#include "EntityFactory.hpp"

#include <systems/CollisionSystem.hpp>
#include <app/InputManager.hpp>

#include "systems/GameplaySystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/GridMapSystem.hpp"
#include "systems/AudioSystem.hpp"

#include "systems/AISystem.hpp"

#include <components/RenderComponents.hpp> // For Motion component
#include <app/GenerateRandomTrees.hpp>
#include <random>

#include "systems/ProceduralGenerationSystem.hpp"

World::World() : m_registry(Registry::get_instance()), m_audioSystem(AudioSystem::get_instance()) {}

World::~World() = default;

void World::restart_game() {
    std::cout << "Restarting game..." << std::endl;

    Registry& registry = Registry::get_instance();
    registry.clear_all_components();

    // Create Player
    auto player = EntityFactory::create_player(glm::vec2(0.0f, 0.0f));
    auto weapon = EntityFactory::create_weapon(glm::vec2(10.0f, 5.0f), 10.0f);
    registry.attackers.get(player).weapon_id = weapon;
    registry.player = player;

    ProceduralGenerationSystem::GenerateDungeon(MAP_WIDTH, MAP_HEIGHT, registry.motions.get(player));

    // create grid map entities
    registry.grid_map = GridMap();
    for (int i = 0; i < int(Globals::update_distance) * 2; i++) {
        registry.grid_map.grid_boxes.push_back(std::vector<GridMap::GridBox>());
        for (int j = 0; j < int(Globals::update_distance) * 2; j++) {
            registry.grid_map.grid_boxes[i].push_back(GridMap::GridBox());
        }
    }
}


void World::demo_init() {
    // Initialize, load sounds and play background music
    m_audioSystem.initialize();
    m_audioSystem.load_all_sound();
    m_audioSystem.start_music();

    restart_game();

    // // Bottom wall (with entrance in the middle)
    // for (int i = 0; i < 5; ++i) {
    //     glm::vec2 pos = glm::vec2(-14.0f + i * 2.0f, -14.0f);
    //     EntityFactory::create_wall(pos, 0.0f);
    // }
    // for (int i = 0; i < 5; ++i) {
    //     glm::vec2 pos = glm::vec2(6.0f + i * 2.0f, -14.0f);
    //     EntityFactory::create_wall(pos, 0.0f);
    // }
    // // Top wall
    // for (int i = 0; i < 15; ++i) {
    //     glm::vec2 pos = glm::vec2(-14.0f + i * 2.0f, 14.0f);
    //
    //     EntityFactory::create_wall(pos, 0.0f);
    // }
    // // Left wall
    // for (int i = 0; i < 14; ++i) {
    //     glm::vec2 pos = glm::vec2(-14.0f, -12.0f + i * 2.0f);
    //     EntityFactory::create_wall(pos, PI / 2.0f);
    // }
    // // Right wall
    // for (int i = 0; i < 14; ++i) {
    //     glm::vec2 pos = glm::vec2(14.0f, -12.0f + i * 2.0f);
    //     EntityFactory::create_wall(pos, PI / 2.0f);
    // }
    //
    // // Place a tree and some enemies
    // std::vector<glm::vec2> trees = GenerateSomeTree::generateNonOverlappingTrees(50, MAP_WIDTH, MAP_HEIGHT, 2.0f);
    // unsigned int i = 0;
    // for (auto& tree_pos : trees) {
    //     if (glm::length(tree_pos) <= 20 || (++i % 4 == 0)) {
    //         if (glm::length(tree_pos) >= 23) {
    //             Entity enemy = EntityFactory::create_enemy(tree_pos);
    //             auto enemy_weapon = EntityFactory::create_weapon(tree_pos, 5.0f, enemy, 0.5f);
    //             m_registry.attackers.get(enemy).weapon_id = enemy_weapon;
    //         }
    //         continue;
    //     }
    //     EntityFactory::create_tree(tree_pos);
    // }
}


void World::step(float elapsed_ms) {
    // TODO: Update the game world
    // 1. Update physics
    GridMapSystem::update_grid_map();

    PhysicsSystem::step(elapsed_ms);
    PhysicsSystem::update_interpolations();

    CollisionSystem::check_collisions();
    CollisionSystem::handle_collisions();

    m_audioSystem.handle_audio_per_frame();

    AISystem::AI_step();

    InputManager::handle_inputs_per_frame();

    GameplaySystem::update_cooldowns(elapsed_ms);
    GameplaySystem::update_regen_stats(elapsed_ms);
    GameplaySystem::update_projectile_range(elapsed_ms);
    GameplaySystem::update_near_player_camera();


    enforce_boundaries(m_registry.player);
}

void World::enforce_boundaries(Entity entity) {
    if (m_registry.motions.has(entity)) {
        Motion& motion = m_registry.motions.get(entity);
        
        // Adjust these values based on the actual visible boundaries of your map
        const float LEFT_BOUND = -MAP_WIDTH / 2.0f;  // Assuming the map is centered
        const float RIGHT_BOUND = MAP_WIDTH / 2.0f;
        const float TOP_BOUND = MAP_HEIGHT / 2.0f;
        const float BOTTOM_BOUND = -MAP_HEIGHT / 2.0f;

        // Enforce X-axis boundaries
        if (motion.position.x < LEFT_BOUND) {
            motion.position.x = LEFT_BOUND;
        } else if (motion.position.x > RIGHT_BOUND) {
            motion.position.x = RIGHT_BOUND;
        }

        // Enforce Y-axis boundaries
        if (motion.position.y < BOTTOM_BOUND) {
            motion.position.y = BOTTOM_BOUND;
        } else if (motion.position.y > TOP_BOUND) {
            motion.position.y = TOP_BOUND;
        }
    }
}
