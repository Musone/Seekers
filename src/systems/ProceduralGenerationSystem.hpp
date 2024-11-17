#pragma once

#include <iostream>
#include <random>
#include "../ecs/Entity.hpp"
#include "../components/Components.hpp"
#include "../ecs/Registry.hpp"
#include "app/EntityFactory.hpp"
#include "utils/Common.hpp"


namespace ProceduralGenerationSystem {
    struct Room {
        glm::vec2 position;    // center of the room
        glm::vec2 size;

        bool operator==(const Room& other) const {return position == other.position && size == other.size;}
    };

    struct Hallway {
        unsigned int room1, room2;  // the index of the two rooms the hallway is connecting
        float distance;             // the distance of the two rooms
    };

    struct WallPosLen {
        int startX, startY; // Starting position of the wall
        int length;         // Length of the wall
        bool horizontal;    // Orientation: true if horizontal, false if vertical
    };

    // Disjoint set (Union-Find) functions for Kruskal's algorithm
    struct DisjointSet {
        std::vector<int> parent, rank;

        DisjointSet(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; ++i) parent[i] = i;
        }

        int find(int u) {
            if (u != parent[u]) parent[u] = find(parent[u]);
            return parent[u];
        }

        void unite(int u, int v) {
            u = find(u);
            v = find(v);
            if (u != v) {
                if (rank[u] < rank[v]) {
                    parent[u] = v;
                } else if (rank[u] > rank[v]) {
                    parent[v] = u;
                } else {
                    parent[v] = u;
                    rank[u]++;
                }
            }
        }
    };

    inline bool do_rooms_overlap(const Room& room1, const Room& room2) {
        float left1 = room1.position.x - room1.size.x/2.0f;
        float right1 = room1.position.x + room1.size.x/2.0f;
        float up1 = room1.position.y + room1.size.y/2.0f;
        float down1 = room1.position.y - room1.size.y/2.0f;
        float left2 = room2.position.x - room2.size.x/2.0f;
        float right2 = room2.position.x + room2.size.x/2.0f;
        float up2 = room2.position.y + room2.size.y/2.0f;
        float down2 = room2.position.y - room2.size.y/2.0f;

        bool collides_x = right1 >= left2 && left1 <= right2;
        bool collides_y = down1 <= up2 && up1 >= down2;

        return collides_x && collides_y;
    }

    inline bool is_overlapping(const Room& room, const std::vector<Room>& rooms) {
        for (const auto& existing_room : rooms) {
            if (do_rooms_overlap(room, existing_room)) {
                return true;
            }
        }
        return false;
    }

    inline std::vector<Room> generate_rooms(int map_width, int map_height, std::vector<Room>& rooms) {
        int min_room_size = 30;
        int max_room_size = 60;
        int room_count = map_width * map_height / (max_room_size * max_room_size);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> pos_x_dist((-map_width + max_room_size) / 2, (map_width - max_room_size) / 2);
        std::uniform_int_distribution<> pos_y_dist((-map_height + max_room_size) / 2, (map_height - max_room_size) / 2);
        std::uniform_int_distribution<> size_dist(min_room_size, max_room_size);

        // Attempt to create rooms
        for (int i = 0; i < room_count; ++i) {
            Room room;
            room.size.x = size_dist(gen);
            room.size.y = size_dist(gen);
            room.position.x = pos_x_dist(gen);
            room.position.y = pos_y_dist(gen);

            // Retry until a non-overlapping room is found or skip if it fails too many times
            int retries = 0;
            while (is_overlapping(room, rooms) && retries < 10) {
                room.position.x = pos_x_dist(gen);
                room.position.y = pos_y_dist(gen);
                ++retries;
            }

            // Add room to the list if no overlap is found
            if (retries < 10) {
                rooms.push_back(room);
            }
        }

        return rooms;
    }

    inline std::vector<Hallway> generate_hallways(const std::vector<Room>& rooms) {
        std::vector<Hallway> hallways;

        int n = rooms.size();
        for (unsigned int i = 0; i < n; ++i) {
            for (unsigned int j = i + 1; j < n; ++j) {
                hallways.push_back({i, j, glm::distance(rooms[i].position, rooms[j].position)});
            }
        }

        auto compare_hallway = [](const Hallway& h1, const Hallway& h2) {
            return h1.distance < h2.distance;
        };
        std::sort(hallways.begin(), hallways.end(), compare_hallway);

        // Kruskal's algorithm to form MST
        DisjointSet ds(n);
        std::vector<Hallway> mst;
        for (const auto& hallway : hallways) {
            if (ds.find(hallway.room1) != ds.find(hallway.room2)) {
                ds.unite(hallway.room1, hallway.room2);
                mst.push_back(hallway);
            }
        }
        return mst;
    }

    inline void connect_rooms(const std::vector<Room>& rooms, const std::vector<Hallway>& hallways, std::vector<std::vector<char>>& map, int map_width, int map_height) {
        int min_hallway_width = 5;
        std::random_device rd;
        std::mt19937 gen(rd());

        for (const auto& room : rooms) {
            for (int y = room.position.y - room.size.y/2; y < room.position.y + room.size.y/2; ++y) {
                for (int x = room.position.x - room.size.x/2; x < room.position.x + room.size.x/2; ++x) {
                    map[-y+map_height/2][x+map_width/2] = 'R';
                }
            }
        }

        std::vector<Room> hallway_rooms;
        for (const auto& hallway : hallways) {
            const Room& room1 = rooms[hallway.room1];
            const Room& room2 = rooms[hallway.room2];

            float left1 = room1.position.x - room1.size.x/2.0f;
            float right1 = room1.position.x + room1.size.x/2.0f;
            float up1 = room1.position.y + room1.size.y/2.0f;
            float down1 = room1.position.y - room1.size.y/2.0f;
            float left2 = room2.position.x - room2.size.x/2.0f;
            float right2 = room2.position.x + room2.size.x/2.0f;
            float up2 = room2.position.y + room2.size.y/2.0f;
            float down2 = room2.position.y - room2.size.y/2.0f;
            bool x_collides = right1 >= left2 && left1 <= right2;
            bool y_collides = down1 <= up2 && up1 >= down2;
            std::vector<float> left_right = {left1, right1, left2, right2};
            std::vector<float> up_down = {up1, down1, up2, down2};
            std::sort(left_right.begin(), left_right.end());
            std::sort(up_down.begin(), up_down.end());

            if (x_collides && left_right[2] - left_right[1] >= min_hallway_width) {
                // up-down hallway
                std::uniform_int_distribution<> hallway_w_dist(min_hallway_width, left_right[2] - left_right[1]);
                // get which room is higher and which is lower
                float upper_down = up_down[2];
                float lower_up = up_down[1];
                // create "room" for hallway
                Room hallway_room;
                hallway_room.size.x = hallway_w_dist(gen);
                hallway_room.size.y = upper_down - lower_up;
                hallway_room.position.x = left_right[1] + hallway_room.size.x/2.0f;
                hallway_room.position.y = lower_up + hallway_room.size.y/2.0f;
                hallway_rooms.push_back(hallway_room);
            } else if (y_collides && up_down[2] - up_down[1] >= min_hallway_width) {
                std::uniform_int_distribution<> hallway_w_dist(min_hallway_width, up_down[2] - up_down[1]);
                // get which room is higher and which is lower
                float lefter_right = left_right[1];
                float righter_left = left_right[2];
                // create "room" for hallway
                Room hallway_room;
                hallway_room.size.x = righter_left - lefter_right;
                hallway_room.size.y = hallway_w_dist(gen);
                hallway_room.position.x = lefter_right + hallway_room.size.x/2.0f;
                hallway_room.position.y = up_down[1] + hallway_room.size.y/2.0f;
                hallway_rooms.push_back(hallway_room);
            } else {
                std::uniform_int_distribution<> hallway_w_dist(min_hallway_width, fmin(fmin(room1.size.x, room1.size.y), fmin(room2.size.x, room2.size.y)));
                int hallway_w = hallway_w_dist(gen);

                // there must be a better way to do this but I don't care
                // horizontal one
                Room hallway_room1;
                // vertical one
                Room hallway_room2;
                if (room1.position.y < room2.position.y) {
                    if (room1.position.x < room2.position.x) {
                        hallway_room1.size.x = left2 - right1 + hallway_w;
                        hallway_room1.size.y = hallway_w;
                        hallway_room2.size.x = hallway_w;
                        hallway_room2.size.y = down2 - up1 + hallway_w;
                        hallway_room1.position.x = right1 + hallway_room1.size.x/2;
                        hallway_room1.position.y =  up1 - hallway_w/2;
                        hallway_room2.position.x = left2 + hallway_w/2;
                        hallway_room2.position.y = down2 - hallway_room2.size.y/2;
                    } else {
                        hallway_room1.size.x = left1 - right2 + hallway_w;
                        hallway_room1.size.y = hallway_w;
                        hallway_room2.size.x = hallway_w;
                        hallway_room2.size.y = down2 - up1 + hallway_w;
                        hallway_room1.position.x = left1 - hallway_room1.size.x/2;
                        hallway_room1.position.y = up1 - hallway_w/2;
                        hallway_room2.position.x = right2 - hallway_w/2;
                        hallway_room2.position.y = down2 - hallway_room2.size.y/2;
                    }
                } else {
                    if (room1.position.x < room2.position.x) {
                        hallway_room1.size.x = left2 - right1 + hallway_w;
                        hallway_room1.size.y = hallway_w;
                        hallway_room2.size.x = hallway_w;
                        hallway_room2.size.y = down1 - up2 + hallway_w;
                        hallway_room1.position.x = left2 - hallway_room1.size.x/2;
                        hallway_room1.position.y = up2 - hallway_w/2;
                        hallway_room2.position.x = right1 - hallway_w/2;
                        hallway_room2.position.y = down1 - hallway_room2.size.y/2;
                    } else {
                        hallway_room1.size.x = left1 - right2 + hallway_w;
                        hallway_room1.size.y = hallway_w;
                        hallway_room2.size.x = hallway_w;
                        hallway_room2.size.y = down1 - up2 + hallway_w;
                        hallway_room1.position.x = right2 + hallway_room1.size.x/2;
                        hallway_room1.position.y = up2 - hallway_w/2;
                        hallway_room2.position.x = left1 + hallway_w/2;
                        hallway_room2.position.y = down1 - hallway_room2.size.y/2;
                    }
                }
                hallway_rooms.push_back(hallway_room1);
                hallway_rooms.push_back(hallway_room2);
            }
        }

        for (const auto& room : hallway_rooms) {
            for (int y = room.position.y - room.size.y/2; y < room.position.y + room.size.y/2; ++y) {
                for (int x = room.position.x - room.size.x/2; x < room.position.x + room.size.x/2; ++x) {
                    map[-y+map_height/2][x+map_width/2] = 'H';
                }
            }
        }
    }

    inline bool is_in_bounds(int x, int y, int width, int height) {
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    inline void place_walls_on_map(std::vector<std::vector<char>>& map) {
        int height = map.size();
        int width = map[0].size();
        std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1},
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
        };
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (map[y][x] == 'R' || map[y][x] == 'H') {
                    for (const auto& kv : directions) {
                        auto dx = kv.first;
                        auto dy = kv.second;
                        int nx = x + dx;
                        int ny = y + dy;
                        if (is_in_bounds(nx, ny, width, height) && map[ny][nx] == '.') {
                            map[ny][nx] = 'W';
                        }
                    }
                }
            }
        }
    }

    inline void create_walls(Registry& registry, const std::vector<std::vector<char>>& map) {
        std::vector<WallPosLen> walls;
        int height = map.size();
        int width = map[0].size();

        // horizontal
        for (int y = 0; y < height; ++y) {
            int startX = -1;
            int length = 0;

            for (int x = 0; x < width; ++x) {
                if (map[y][x] == 'W') {
                    if (startX == -1) { // Start a new wall segment
                        startX = x;
                        length = 1;
                    } else {
                        length++; // Extend the current wall segment
                    }
                } else if (startX != -1) { // End of a wall segment
                    if (length > 1) {walls.push_back({startX, y, length, true});}
                    startX = -1;
                    length = 0;
                }
            }
            // Check if a wall segment ends at the row's end
            if (startX != -1 && length > 1) {
                walls.push_back({startX, y, length, true});
            }
        }

        // vertical
        for (int x = 0; x < width; ++x) {
            int startY = -1;
            int length = 0;

            for (int y = 0; y < height; ++y) {
                if (map[y][x] == 'W') {
                    if (startY == -1) { // Start a new wall segment
                        startY = y;
                        length = 1;
                    } else {
                        length++; // Extend the current wall segment
                    }
                } else if (startY != -1) { // End of a wall segment
                    if (length > 1) {walls.push_back({x, startY, length, false});}
                    startY = -1;
                    length = 0;
                }
            }
            // Check if a wall segment ends at the column's end
            if (startY != -1 && length > 1) {
                walls.push_back({x, startY, length, false});
            }
        }

        // translate map indices to coordinates
        for (auto& wall : walls) {
            wall.startX = wall.startX - width/2;
            wall.startY = height/2 - wall.startY;
        }

        // create the walls
        for (auto& wall : walls) {
            float x = wall.startX;
            float y = wall.startY;

            if (wall.horizontal) {
                x += wall.length/2;
                EntityFactory::create_wall(registry, {x, y}, 0, glm::vec2(wall.length, 1.0f));
            } else {
                y -= wall.length/2;
                EntityFactory::create_wall(registry, {x, y}, PI / 2.0f, glm::vec2(wall.length, 1.0f));
            }
        }
    }

    inline Room create_spawn_room(std::vector<Room>& rooms, int map_width, int map_height) {
        Room room;
        room.size = glm::vec2(20, 20);
        room.position = glm::vec2((-map_width + room.size.x ) / 2 + 1, (-map_height + room.size.y) / 2 + 2);
        rooms.push_back(room);
        return room;
    }

    inline bool enemies_objects_overlap(const std::vector<std::pair<int, int>>& positions, int x, int y) {
        int empty_radius = 4;
        for (const auto& pos : positions) {
            if ((pos.first-x)*(pos.first-x) + (pos.second-y)*(pos.second-y) < empty_radius * empty_radius) {
                return true;
            }
        }
        return false;
    }

    inline void create_enemies_and_objects(Registry& registry, const std::vector<Room>& rooms, const Room& spawn_room) {
        for (const Room& room : rooms) {
            if (room == spawn_room) {continue;}

            std::vector<std::pair<int, int>> enemies_and_objects_pos;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> pos_x_dist(room.position.x - room.size.x / 2 + 2, room.position.x + room.size.x / 2 - 2);
            std::uniform_int_distribution<> pos_y_dist(room.position.y - room.size.y / 2 + 2, room.position.y + room.size.y / 2 - 2);
            std::uniform_int_distribution<> enemy_num_dist(1, room.size.x * room.size.y / 400);
            std::uniform_int_distribution<> object_num_dist(1, room.size.x * room.size.y / 600);
            std::uniform_int_distribution<> enemy_type_dist(0, (int)ENEMY_TYPE::ENEMY_TYPE_COUNT - 1);

            int enemy_num = enemy_num_dist(gen);
            for (int i = 0; i < enemy_num; i++) {
                int x = pos_x_dist(gen);
                int y = pos_y_dist(gen);
                while (enemies_objects_overlap(enemies_and_objects_pos, x, y)) {
                    x = pos_x_dist(gen);
                    y = pos_y_dist(gen);
                }
                EntityFactory::create_enemy(registry, {x, y}, (ENEMY_TYPE)enemy_type_dist(gen));
                enemies_and_objects_pos.push_back({x, y});
            }
            int object_num = object_num_dist(gen);
            for (int i = 0; i < object_num; i++) {
                int x = pos_x_dist(gen);
                int y = pos_y_dist(gen);
                while (enemies_objects_overlap(enemies_and_objects_pos, x, y)) {
                    x = pos_x_dist(gen);
                    y = pos_y_dist(gen);
                }
                EntityFactory::create_tree(registry, {x, y});
                enemies_and_objects_pos.push_back({x, y});
            }
        }
    }

    inline void generate_dungeon(Registry& registry, int map_width, int map_height, Motion& player_motion) {
        std::vector<std::vector<char>> map(map_height, std::vector<char>(map_width, '.'));

        std::vector<Room> rooms;
        Room spawn_room = create_spawn_room(rooms, map_width, map_height);
        player_motion.position = spawn_room.position;
        generate_rooms(map_width, map_height, rooms);
        std::vector<Hallway> hallways = generate_hallways(rooms);
        connect_rooms(rooms, hallways, map, map_width, map_height);
        place_walls_on_map(map);
        create_walls(registry, map);

        create_enemies_and_objects(registry, rooms, spawn_room);

        // print map
        for (const auto& row : map) {
            for (const auto& cell : row) {
                std::cout << cell;
            }
            std::cout << '\n';
        }
    }
}