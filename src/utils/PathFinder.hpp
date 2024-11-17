#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>

#include <components/Components.hpp>


bool is_valid_not_occupied_in_grid(
        const std::vector<std::vector<GridMap::GridBox>>& grid,
        int i, int j
) {
    return 0 <= i && i < grid.size() && 0 <= j && j < grid[0].size() && !grid[i][j].is_occupied && grid[i][j].distance != -1;
}

void update_next_position(
        std::vector<std::vector<GridMap::GridBox>>& grid,
        glm::vec2 next_to_check_position,
        int& minimum_distance_found,
        glm::vec2& next_position
) {
    if (is_valid_not_occupied_in_grid(grid, next_to_check_position.x, next_to_check_position.y)) {
        if (minimum_distance_found == -1 ||
                grid[next_to_check_position.x][next_to_check_position.y].distance < minimum_distance_found) {
            minimum_distance_found = grid[next_to_check_position.x][next_to_check_position.y].distance;
            next_position = {next_to_check_position.x, next_to_check_position.y};
        }
    }
}

glm::vec2 get_next_point_of_path_to_player(
        std::vector<std::vector<GridMap::GridBox>>& grid,
        int start_i, int start_j,
        float self_radius
) {
    int max_self_radius = int(std::ceil(self_radius)) + 1;
    int minimum_distance_found = -1;
    glm::vec2 next_position = {start_i, start_j};
    std::vector<glm::vec2> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (auto& direction : directions) {
        int i_diff = direction.x;
        int j_diff = direction.y;
        update_next_position(grid, {start_i + i_diff * max_self_radius, start_j + j_diff * max_self_radius}, minimum_distance_found, next_position);
    }

    if (minimum_distance_found <= 5) {
        return {start_i, start_j};
    }

    return next_position;
}

glm::vec2 get_grid_map_coordinates(Motion& motion) {
    Registry& registry = MapManager::get_instance().get_active_registry();
    Motion& player_motion = registry.motions.get(registry.player);
    glm::vec2 distance = motion.position - player_motion.position;
    int grid_i = int(Globals::update_distance) - int(std::floor(distance.y));
    int grid_j = int(Globals::update_distance) + int(std::floor(distance.x));
    return {grid_i, grid_j};
}

glm::vec2 get_position_from_grid_map_coordinates(int i, int j) {
    Registry& registry = MapManager::get_instance().get_active_registry();
    Motion& player_motion = registry.motions.get(registry.player);
    glm::vec2 position = player_motion.position;
    position.x += j - int(Globals::update_distance);
    position.y += int(Globals::update_distance) - i;
    return position;
}

bool can_see(
        std::vector<std::vector<GridMap::GridBox>>& grid,
        int current_x, int current_y,
        int self_radius,
        int target_x,
        int target_y
) {
    glm::vec2 dir = Common::normalize(glm::vec2({target_x, target_y}) - glm::vec2({current_x, current_y}));
    float next_x = current_x + dir.x;
    float next_y = current_y + dir.y;
    int occupied_cells = 0;
    while (glm::length(glm::vec2({target_x, target_y}) - glm::vec2({next_x, next_y})) >= 1) {
        bool is_occupied = grid[int(std::round(next_x))][int(std::round(next_y))].is_occupied;
        if (is_occupied) {
            occupied_cells++;
        }
        if (occupied_cells > self_radius + 2) {
            return false;
        }
        next_x += dir.x;
        next_y += dir.y;
    }
    return true;
}