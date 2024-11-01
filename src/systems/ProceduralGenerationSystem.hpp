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
    };

    struct Hallway {
        unsigned int room1, room2;  // the index of the two rooms the hallway is connecting
        float distance;             // the distance of the two rooms
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

    inline std::vector<Room> generate_rooms(int map_width, int map_height) {
        int min_room_size = 20;
        int max_room_size = 40;
        int room_count = map_width * map_height / (max_room_size * max_room_size);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> pos_x_dist((-map_width + max_room_size) / 2, (map_width - max_room_size) / 2);
        std::uniform_int_distribution<> pos_y_dist((-map_height + max_room_size) / 2, (map_height - max_room_size) / 2);
        std::uniform_int_distribution<> size_dist(min_room_size, max_room_size);

        std::vector<Room> rooms;

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
            std::vector<float> up_down = {up1, down2, up2, down2};
            std::sort(left_right.begin(), left_right.end());
            std::sort(up_down.begin(), up_down.end());

            if (x_collides && left_right[2] - left_right[1] >= min_hallway_width) {
                // TODO: FIX THIS SHIIIIT
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
                // TODO: left-right hallway
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
                // TODO: L-shaped hallway
                std::cout << "L-shaped hallway" << std::endl;
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

    inline void GenerateDungeon(int map_width, int map_height) {
        Registry& registry = Registry::get_instance();

        std::vector<std::vector<char>> map(map_height, std::vector<char>(map_width, '.'));
        std::vector<Room> rooms = generate_rooms(map_width, map_height);
        std::vector<Hallway> hallways = generate_hallways(rooms);
        connect_rooms(rooms, hallways, map, map_width, map_height);

        for (const auto& room : rooms) {
            glm::vec2 left_wall_position = {room.position.x - room.size.x / 2.0f + 1, room.position.y};
            glm::vec2 right_wall_position = {room.position.x + room.size.x / 2.0f - 1, room.position.y};
            glm::vec2 top_wall_position = {room.position.x, room.position.y - room.size.y / 2.0f + 1};
            glm::vec2 bottom_wall_position = {room.position.x, room.position.y + room.size.y / 2.0f - 1};
            EntityFactory::create_no_collision_wall(left_wall_position, PI / 2.0f, glm::vec2(room.size.y, 2.0f));
            EntityFactory::create_no_collision_wall(right_wall_position, PI / 2.0f, glm::vec2(room.size.y, 2.0f));
            EntityFactory::create_no_collision_wall(top_wall_position, 0.f, glm::vec2(room.size.x, 2.0f));
            EntityFactory::create_no_collision_wall(bottom_wall_position, 0.f, glm::vec2(room.size.x, 2.0f));
        }

        // print map
        for (const auto& row : map) {
            for (const auto& cell : row) {
                std::cout << cell;
            }
            std::cout << '\n';
        }
    }
}