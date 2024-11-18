#pragma once
#include <vector>
#include <random>
#include <glm/glm.hpp>

#define PI 3.1415926535


namespace OpenWorldMapCreatorSystem {
    inline bool is_position_valid(const glm::vec2& new_pos, const std::vector<glm::vec2>& existing_positions, float min_distance) {
        for (const auto& pos : existing_positions) {
            if (glm::distance(new_pos, pos) < min_distance) {
                return false;
            }
        }
        return true;
    }

    inline void create_forest(Registry& registry, const glm::vec2& center_position, int num_trees = 100, float min_distance = 30.0f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        float forest_radius = min_distance * sqrt(num_trees) * 0.5f;

        std::uniform_real_distribution<float> dist_radius(0.0f, forest_radius);
        std::uniform_real_distribution<float> dist_angle(0.0f, 2.0f * PI);
        std::uniform_real_distribution<float> dist_rotation(0.0f, 2.0f * PI);

        std::vector<glm::vec2> tree_positions;
        int attempts = 1000; // prevent infinite loops
        int trees_created = 0;

        while (trees_created < num_trees && attempts > 0) {
            float radius = dist_radius(gen);
            float angle = dist_angle(gen);
            float tree_rotation = dist_rotation(gen);
            
            glm::vec2 offset(radius * cos(angle), radius * sin(angle));

            glm::vec2 new_pos = center_position + offset;

            if (is_position_valid(new_pos, tree_positions, min_distance)) {
                EntityFactory::create_tree(registry, new_pos, tree_rotation);
                tree_positions.push_back(new_pos);
                trees_created++;
            }

            attempts--;
        }
    }

    inline void populate_open_world_map(Registry& registry) {
        // TODO: add everything here Sam
        //EntityFactory::create_tree(registry, glm::vec2(0, 0)); //example
        create_forest(registry, glm::vec2(0.f, 0.f));
        // create_forest(registry, glm::vec2(-20.f, 160.f));
        // create_forest(registry, glm::vec2(-180.f, 160.f), 150);
        // EntityFactory::create_rock(registry, {50, 50});                         // example of rock
        // EntityFactory::create_tree(registry, glm::vec2(40.f, 40.f), PI);     // example of tree with angle
    }
};