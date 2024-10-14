#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <ctime>

namespace GenerateSomeTree {

    bool checkCollision(const glm::vec2& a, const glm::vec2& b, float radius) {
        return glm::distance(a, b) < (2 * radius);
    }

    std::vector<glm::vec2> generateNonOverlappingTrees(int count, float boundary_width, float boundary_height, float TREE_RADIUS) {
        std::vector<glm::vec2> trees;
        
        std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_real_distribution<float> distX(-boundary_width/2 + TREE_RADIUS, boundary_width/2 - TREE_RADIUS);
        std::uniform_real_distribution<float> distY(-boundary_height/2 + TREE_RADIUS, boundary_height/2 - TREE_RADIUS);

        const int MAX_ATTEMPTS = 100;  // Maximum attempts to place a tree

        for (int i = 0; i < count; ++i) {
            glm::vec2 newTree;
            bool validPosition = false;
            int attempts = 0;

            while (!validPosition && attempts < MAX_ATTEMPTS) {
                newTree.x = distX(rng);
                newTree.y = distY(rng);

                validPosition = true;
                for (const auto& existingTree : trees) {
                    if (checkCollision(newTree, existingTree, TREE_RADIUS)) {
                        validPosition = false;
                        break;
                    }
                }
                attempts++;
            }

            if (validPosition) {
                trees.push_back(newTree);
            } else {
                // If we couldn't place a tree after MAX_ATTEMPTS, we break the loop
                // This prevents an infinite loop if the map is too crowded
                break;
            }
        }

        return trees;
    }
};