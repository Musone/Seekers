#include "World.h"
#include "EntityFactory.hpp"

#include <systems/CollisionSystem.hpp>
#include <app/InputManager.hpp>

#include "systems/GameplaySystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include <random>

#include <components/RenderComponents.hpp> // For Motion component

World::World() : m_registry(Registry::get_instance()), m_audioSystem(AudioSystem::get_instance()) {}

World::~World() = default;

void World::demo_init() {
    // Initialize, load sounds and play background music
    m_audioSystem.initialize();
    m_audioSystem.load_music(audio_path("music.wav"));
    m_audioSystem.load_sound_effect(audio_path("footstep.wav"));
    m_audioSystem.load_sound_effect(audio_path("teleport.wav"));
    m_audioSystem.load_sound_effect(audio_path("attack.wav"));
    m_audioSystem.set_music_volume(16);
    m_audioSystem.play_music(-1);

    // Create Player
    auto player = EntityFactory::create_player(glm::vec2(0.0f, 0.0f));
    auto weapon = EntityFactory::create_weapon(glm::vec2(10.0f, 5.0f), 10.0f, player);
    m_registry.attackers.get(player).weapon_id = weapon;
    m_players.push_back(player);
    m_registry.player = player;

    // Create some enemies
    std::random_device rd;
    std::mt19937 eng(rd());
    std:: uniform_real_distribution<float> distr_pos(-70.0f, 70.0f);
    for (int i = 0; i < 100; ++i) {
        // glm::vec2 pos = glm::vec2(10.0f + i * 10.0f, 10.0f);
        // auto enemy = EntityFactory::create_enemy(pos);
        // auto enemy_weapon = EntityFactory::create_weapon(pos, 5.0f, enemy);
        // m_registry.attackers.get(enemy).weapon_id = enemy_weapon;
        // m_enemies.push_back(enemy);
        glm::vec2 pos = glm::vec2(distr_pos(eng), distr_pos(eng));
        if (pos.x >= -70.f && pos.x <= 70.f && pos.y >= -70.f && pos.y <= 70.f) {
            auto enemy = EntityFactory::create_enemy(pos);
            auto enemy_weapon = EntityFactory::create_weapon(pos, 5.0f, enemy);
            m_registry.attackers.get(enemy).weapon_id = enemy_weapon;
            m_enemies.push_back(enemy);
        }
    }

    // Create wall around the whole map
    // horizontal
    for (int i = -74; i < 75; ++i) {
        EntityFactory::create_wall(glm::vec2(i, 74.f), 0.0f);
        EntityFactory::create_wall(glm::vec2(i, -74.f), 0.0f);
    }

    //vertical
    for (int i = -74; i < 75; ++i) {
        EntityFactory::create_wall(glm::vec2(74.f, i), PI / 2.0f);
        EntityFactory::create_wall(glm::vec2(-74.f, i), PI / 2.0f);
    }

    // Create some test walls
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(5.0f + i * 2.0f, 5.0f);
        EntityFactory::create_wall(pos, 0.0f);
    }
    for (int i = 0; i < 5; ++i) {
        glm::vec2 pos = glm::vec2(5.0f, (i + 1) * 2.0f + 5.0f);
        EntityFactory::create_wall(pos, PI / 2.0f);
    }

    // random tree walls generation
    std::uniform_real_distribution<float> distr_x(-70.f, 70.f);
    std::uniform_int_distribution<int> distr_trees(3, 6);

    // y offsets
    float y_arr[] = {-65.f, -50.f, -35.f, -20.f, -5.f, 10.f, 25.f, 40.f, 55.f, 70.f};

    for (int i = 0; i <sizeof(y_arr) / sizeof(y_arr[0]); ++i) {
        float start_x = distr_x(eng);
        int num_trees = distr_trees(eng);

        for (int j = 0; j < num_trees; ++j) {
            float x_pos = start_x + j * 3.3f;
            float x_neg = start_x - j * 3.3f;

            if (x_pos >= -70.f && x_pos <= 70.f) {
                EntityFactory::create_tree(glm::vec2(x_pos, y_arr[i]));
            }

            if (x_neg >= -70.f && x_neg <= 70.f) {
                EntityFactory::create_tree(glm::vec2(x_neg, y_arr[i]));
            }
        }

        float column_x = start_x + (std::uniform_int_distribution<int>(0, num_trees - 1)(eng) * 3.3f);
        int num_column_trees = distr_trees(eng);

        for (int k = 0; k < num_column_trees; ++k) {
            float y_pos = y_arr[i] + k * 3.3f;
            float y_neg = y_arr[i] - k * 3.3f;

            if (column_x >= -70.f && column_x <= 70.f) {
                if (y_pos >= -70.f && y_pos <= 70.f) {
                    EntityFactory::create_tree(glm::vec2(column_x, y_pos));
                }
                if (y_neg >= -70.f && y_neg <= 70.f) {
                    EntityFactory::create_tree(glm::vec2(column_x, y_neg));
                }
            }
        }
    }

    // Create some tree cluster

    // number of cluster
    std::uniform_int_distribution<int> cluster_count_dist(20, 30);
    // offset from cluster center
    std::uniform_real_distribution<float> distr_offset(-5.f, 5.f);
    // number of tree per clusters
    std::uniform_int_distribution<int> cluster_trees(10, 20);
    // cluster center
    std::uniform_real_distribution<float> cluster_center(-70.f, 70.f);
    
    int num_clusters = cluster_count_dist(eng);

    for (int c = 0; c < cluster_count_dist(eng); ++c) {
        glm::vec2 center(cluster_center(eng), cluster_center(eng));

        int tree_cluster_num = cluster_trees(eng);

        for (int i = 0; i < tree_cluster_num; ++i) {
            float offset_x = distr_offset(eng);
            float offset_y = distr_offset(eng);

            glm::vec2 tree_pos = center + glm::vec2(offset_x, offset_y);

            if (tree_pos.x >= -70.f && tree_pos.x <= 70.f && tree_pos.y >= -70.f && tree_pos.y <= 70.f) {
                EntityFactory::create_tree(tree_pos);
            }
        }
    }
}

void World::step(float elapsed_ms) {
    // TODO: Update the game world
    // 1. Update physics
    PhysicsSystem::step(elapsed_ms);
    PhysicsSystem::update_interpolations();

    CollisionSystem::check_collisions();
    CollisionSystem::handle_collisions();

    InputManager::handle_inputs_per_frame();

    GameplaySystem::update_cooldowns(elapsed_ms);
    GameplaySystem::update_projectile_range(elapsed_ms);

    // Enforce boundaries for players
    for (const auto& player : m_players) {
        enforce_boundaries(player);
    }
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
