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

    inline bool do_rooms_overlap(const Room& room1, const Room& room2) {
        float left1 = room1.position.x - room1.size.x/2.0f;
        float right1 = room1.position.x + room1.size.x/2.0f;
        float up1 = room1.position.y - room1.size.y/2.0f;
        float down1 = room1.position.y + room1.size.y/2.0f;
        float left2 = room2.position.x - room2.size.x/2.0f;
        float right2 = room2.position.x + room2.size.x/2.0f;
        float up2 = room2.position.y - room2.size.y/2.0f;
        float down2 = room2.position.y + room2.size.y/2.0f;

        bool collides_x = right1 >= left2 && left1 <= right2;
        bool collides_y = down1 >= up2 && up1 <= down2;

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

        // // Initialize the map grid
        // std::vector<std::vector<char>> map(map_height, std::vector<char>(map_width, '.'));

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
                // // Mark room area on the map
                // for (int y = room.position.y; y < room.position.y + room.size.y; ++y) {
                //     for (int x = room.position.x; x < room.position.x + room.size.x; ++x) {
                //         map[y][x] = '#';
                //     }
                // }
            }
        }

        return rooms;
    }

    inline void GenerateDungeon(int map_width, int map_height) {
        Registry& registry = Registry::get_instance();

        std::vector<Room> rooms = generate_rooms(map_width, map_height);

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
    }
}