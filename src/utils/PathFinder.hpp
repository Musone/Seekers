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
        int start_x, int start_y,
        float self_radius
) {
    int max_self_radius = int(std::ceil(self_radius)) + 2;
    int minimum_distance_found = -1;
    glm::vec2 next_position = {start_x, start_y};
    std::vector<glm::vec2> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (auto& direction : directions) {
        int x_diff = direction.x;
        int y_diff = direction.y;
        update_next_position(grid, {start_x + x_diff * max_self_radius, start_y + y_diff * max_self_radius}, minimum_distance_found, next_position);
    }

    if (minimum_distance_found <= 5) {
        return {start_x, start_y};
    }

    return next_position;
}

glm::vec2 get_grid_map_coordinates(Motion& motion) {
    Registry& registry = Registry::get_instance();
    Motion& player_motion = registry.motions.get(registry.player);
    glm::vec2 distance = motion.position - player_motion.position;
    int grid_i = int(std::floor(distance.x))
                 + int(Globals::update_distance);
    int grid_j = int(std::floor(distance.y))
                 + int(Globals::update_distance);;
    return {grid_i, grid_j};
}

glm::vec2 get_position_from_grid_map_coordinates(int x, int y) {
    Registry& registry = Registry::get_instance();
    Motion& player_motion = registry.motions.get(registry.player);
    glm::vec2 position = player_motion.position;
    position.x += x - int(Globals::update_distance);
    position.y += y - int(Globals::update_distance);
    return position;
}