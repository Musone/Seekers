#pragma once

#include <glm/vec2.hpp>
#include "ecs/Entity.hpp"
#include <vector>

struct Motion
{
	glm::vec2 position = {0, 0};
	float angle = 0;
	float rotation_velocity = 0;
	glm::vec2 scale = {0, 0};
	glm::vec2 velocity = {0, 0};
	glm::vec2 acceleration = {0, 0};
	float drag = 0;
};

struct MoveWith
{
	unsigned int following_entity_id;
	MoveWith(unsigned int id) : following_entity_id(id) {}
};

struct RotateWith
{
	unsigned int following_entity_id;
	RotateWith(unsigned int id) : following_entity_id(id) {}
};

enum class ColliderType {
    Circle,
    AABB,
    Mesh
};

struct CircleCollider {
    float radius;
};

struct AABBCollider {
    glm::vec2 min;
    glm::vec2 max;
    
    void update(const glm::vec2& position, const glm::vec2& size) {
        min = position - size/2.0f;
        max = position + size/2.0f;
    }
};

struct MeshCollider {
    std::vector<glm::vec2> vertices;
    float bound_radius;  // For broad phase
};

struct CollisionBounds {
    ColliderType type;
    union {
        CircleCollider circle;
        AABBCollider aabb;
        MeshCollider* mesh;  // Pointer because union can't contain objects with constructors
    };
    
    // Constructors for different types
    static CollisionBounds create_circle(float radius) {
        CollisionBounds bounds;
        bounds.type = ColliderType::Circle;
        bounds.circle.radius = radius;
        return bounds;
    }
    
    static CollisionBounds create_aabb(const glm::vec2& size) {
        CollisionBounds bounds;
        bounds.type = ColliderType::AABB;
        bounds.aabb.min = -size/2.0f;
        bounds.aabb.max = size/2.0f;
        return bounds;
    }
    
    static CollisionBounds create_mesh(const std::vector<glm::vec2>& verts, float bound_radius) {
        CollisionBounds bounds;
        bounds.type = ColliderType::Mesh;
        bounds.mesh = new MeshCollider{verts, bound_radius};
        return bounds;
    }

    ~CollisionBounds() {
        if (type == ColliderType::Mesh && mesh != nullptr) {
            delete mesh;
        }
    }
};

struct Collision {
    Entity other;
    Collision(Entity &other) { this->other = other; }
};