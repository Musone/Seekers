#pragma once

#include <cmath>
#include <globals/Globals.h>
#include "../ecs/Registry.hpp"

namespace GridMapSystem {
    inline void _clear_grid_map() {
        Registry& registry = Registry::get_instance();

        for (auto& grid_row : registry.grid_map.grid_boxes) {
            for (auto& grid_box : grid_row) {
                grid_box.is_occupied = false;
                grid_box.distance = -1;
            }
        }
    }

    inline void update_grid_distances(std::vector<std::vector<GridMap::GridBox>> &grid_boxes, int x, int y) {
        std::vector<std::vector<int>> points;
        grid_boxes[x][y].distance = 0; // Start point
        points.push_back({x, y});

        for (size_t i = 0; i < points.size(); ++i) {
            int cx = points[i][0];
            int cy = points[i][1];
            int current_distance = grid_boxes[cx][cy].distance;

            if (cx - 1 >= 0 && !grid_boxes[cx - 1][cy].is_occupied && grid_boxes[cx - 1][cy].distance == -1) {
                grid_boxes[cx - 1][cy].distance = current_distance + 1;
                points.push_back({cx - 1, cy});
            }
            if (cx + 1 < grid_boxes.size() && !grid_boxes[cx + 1][cy].is_occupied && grid_boxes[cx + 1][cy].distance == -1) {
                grid_boxes[cx + 1][cy].distance = current_distance + 1;
                points.push_back({cx + 1, cy});
            }
            if (cy - 1 >= 0 && !grid_boxes[cx][cy - 1].is_occupied && grid_boxes[cx][cy - 1].distance == -1) {
                grid_boxes[cx][cy - 1].distance = current_distance + 1;
                points.push_back({cx, cy - 1});
            }
            if (cy + 1 < grid_boxes[cx].size() && !grid_boxes[cx][cy + 1].is_occupied && grid_boxes[cx][cy + 1].distance == -1) {
                grid_boxes[cx][cy + 1].distance = current_distance + 1;
                points.push_back({cx, cy + 1});
            }
        }
    }

    inline void update_grid_map() {
        _clear_grid_map();
        Registry& registry = Registry::get_instance();
        Motion& player_motion = registry.motions.get(registry.player);
        for (Entity& e : registry.near_players.entities) {
            if (registry.motions.has(e)) {
                if (registry.player == e) {
                    continue;
                }
                Motion& motion = registry.motions.get(e);
                glm::vec2 distance = motion.position - player_motion.position;
                int grid_i = int(std::floor(distance.x))
                        + int(Globals::update_distance);
                int grid_j = int(std::floor(distance.y))
                        + int(Globals::update_distance);
                if (0 > grid_i || grid_i > int(Globals::update_distance) * 2 - 1) {
                    break;
                }
                else if (0 > grid_j || grid_j > int(Globals::update_distance) * 2 - 1) {
                    break;
                }
                registry.grid_map.grid_boxes[grid_i][grid_j].is_occupied = true;
                if (registry.collision_bounds.has(e)) {
                    const CollisionBounds& box = registry.collision_bounds.get(e);
                    float radius = 0.0f;
                    
                    // Get radius based on collider type
                    if (box.type == ColliderType::Circle) {
                        radius = box.circle.radius;
                    } else if (box.type == ColliderType::Wall) {
                        // For walls, use half the largest dimension of AABB
                        glm::vec2 size = box.wall->aabb.max - box.wall->aabb.min;
                        radius = std::max(size.x, size.y) * 0.5f;
                    } else if (box.type == ColliderType::Mesh) {
                        radius = box.mesh->bound_radius;
                    }

                    // Update grid boxes within radius
                    for (int i = grid_i - int(std::floor(radius)) - 2; 
                         i <= grid_i + int(std::floor(radius)) + 2; i++) {
                        for (int j = grid_j - int(std::floor(radius)) - 2; 
                             j <= grid_j + int(std::floor(radius)) + 2; j++) {
                            if (0 > i || i > int(Globals::update_distance) * 2 - 1) {
                                break;
                            }
                            else if (0 > j || j > int(Globals::update_distance) * 2 - 1) {
                                break;
                            }
                            if (registry.grid_map.grid_boxes[i][j].is_occupied) {
                                break;
                            }
                            registry.grid_map.grid_boxes[i][j].is_occupied = true;
                        }
                    }
                }
            }
        }
        std::vector<std::vector<GridMap::GridBox>> &grid_boxes = Registry::get_instance().grid_map.grid_boxes;
        grid_boxes[int(Globals::update_distance)][int(Globals::update_distance)].distance = 0;
        update_grid_distances(grid_boxes, int(Globals::update_distance), int(Globals::update_distance));

        //                            printmap
        for (int i = 0; i < int(Globals::update_distance) * 2; i++) {
            for (int j = 0; j < int(Globals::update_distance) * 2; j++) {
                if (registry.grid_map.grid_boxes[i][j].is_occupied) {
                    printf("*   ");
                }
                else if (registry.grid_map.grid_boxes[i][j].distance == -1) {
                    printf("-1  ");
                } else if (registry.grid_map.grid_boxes[i][j].distance >= 100) {
                    printf("%d ", registry.grid_map.grid_boxes[i][j].distance);
                }
                else if (registry.grid_map.grid_boxes[i][j].distance >= 10) {
                    printf("%d  ", registry.grid_map.grid_boxes[i][j].distance);
                } else {
                    printf("%d   ", registry.grid_map.grid_boxes[i][j].distance);
                }
            }
            printf("X\n");
        }
        printf("-----------------------------------------------------------------------------\n");
    }
};