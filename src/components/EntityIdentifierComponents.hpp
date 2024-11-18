#pragma once

// This file defines components that specify the type of entity they are associated with

enum class WALL_TYPE
{
    BRICK = 0,
    STONE = BRICK + 1,
    FANCY = STONE + 1,
    WALL_TYPE_COUNT = FANCY + 1
};
const int wall_type_count = (int)WALL_TYPE::WALL_TYPE_COUNT;

struct Wall {
    WALL_TYPE type = WALL_TYPE::WALL_TYPE_COUNT;
};

enum class ENEMY_TYPE
{
    WARRIOR = 0,
    ARCHER = WARRIOR + 1,
    ZOMBIE = ARCHER + 1,
    ENEMY_TYPE_COUNT = ZOMBIE + 1
};
const int enemy_type_count = (int)ENEMY_TYPE::ENEMY_TYPE_COUNT;

struct Enemy {
    ENEMY_TYPE type = ENEMY_TYPE::ENEMY_TYPE_COUNT;
};

enum class STATIC_OBJECT_TYPE
{
    TREE = 0,
    ROCK = TREE + 1,
    STATUE = ROCK + 1,
    BONFIRE = STATUE + 1,
    PORTAL = BONFIRE + 1,
    STATIC_OBJECT_TYPE_COUNT = PORTAL + 1
};
const int static_object_type_count = (int)STATIC_OBJECT_TYPE::STATIC_OBJECT_TYPE_COUNT;

struct StaticObject {
    STATIC_OBJECT_TYPE type = STATIC_OBJECT_TYPE::STATIC_OBJECT_TYPE_COUNT;
};

enum class LIGHT_SOURCE_TYPE {
    MAGIC_ORB = 1,
    LIGHT_SOURCE_TYPE_COUNT = MAGIC_ORB + 1
};

struct LightSource {
    float brightness;
    glm::vec3 pos;
};
