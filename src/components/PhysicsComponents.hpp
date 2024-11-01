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
    Wall,    // Replaces AABB
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

struct LineSegment {
    glm::vec2 start;
    glm::vec2 end;
    glm::vec2 normal;  // Pre-calculated normal vector
};

struct WallCollider {
    AABBCollider aabb;              // For broad-phase and rendering
    std::vector<LineSegment> edges; // For precise collision
};

struct CollisionBounds {
    ColliderType type;
    union {
        CircleCollider circle;
        AABBCollider aabb;
        WallCollider* wall;
        MeshCollider* mesh;
    };

    // Default constructor
    CollisionBounds() : type(ColliderType::Circle), circle{0} {}

    // Copy constructor
    CollisionBounds(const CollisionBounds& other) : type(other.type) {
        switch (type) {
            case ColliderType::Circle:
                circle = other.circle;
                break;
            case ColliderType::AABB:
                aabb = other.aabb;
                break;
            case ColliderType::Wall:
                wall = new WallCollider(*other.wall);
                break;
            case ColliderType::Mesh:
                mesh = new MeshCollider(*other.mesh);
                break;
        }
    }

    // Assignment operator
    CollisionBounds& operator=(const CollisionBounds& other) {
        if (this != &other) {
            cleanup();
            
            type = other.type;
            switch (type) {
                case ColliderType::Circle:
                    circle = other.circle;
                    break;
                case ColliderType::AABB:
                    aabb = other.aabb;
                    break;
                case ColliderType::Wall:
                    wall = new WallCollider(*other.wall);
                    break;
                case ColliderType::Mesh:
                    mesh = new MeshCollider(*other.mesh);
                    break;
            }
        }
        return *this;
    }

    // Move constructor
    CollisionBounds(CollisionBounds&& other) noexcept : type(other.type) {
        switch (type) {
            case ColliderType::Circle:
                circle = other.circle;
                break;
            case ColliderType::AABB:
                aabb = other.aabb;
                break;
            case ColliderType::Wall:
                wall = other.wall;
                other.wall = nullptr;
                break;
            case ColliderType::Mesh:
                mesh = other.mesh;
                other.mesh = nullptr;
                break;
        }
    }

    // Move assignment operator
    CollisionBounds& operator=(CollisionBounds&& other) noexcept {
        if (this != &other) {
            cleanup();
            type = other.type;
            switch (type) {
                case ColliderType::Circle:
                    circle = other.circle;
                    break;
                case ColliderType::AABB:
                    aabb = other.aabb;
                    break;
                case ColliderType::Wall:
                    wall = other.wall;
                    other.wall = nullptr;
                    break;
                case ColliderType::Mesh:
                    mesh = other.mesh;
                    other.mesh = nullptr;
                    break;
            }
        }
        return *this;
    }

    // Helper function for cleanup
    void cleanup() {
        if (type == ColliderType::Wall && wall != nullptr) {
            delete wall;
            wall = nullptr;
        }
        if (type == ColliderType::Mesh && mesh != nullptr) {
            delete mesh;
            mesh = nullptr;
        }
    }

    ~CollisionBounds() {
        cleanup();
    }

    // Factory functions
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
    
    static CollisionBounds create_wall(const glm::vec2& size) {
        CollisionBounds bounds;
        bounds.type = ColliderType::Wall;
        bounds.wall = new WallCollider;
        
        bounds.wall->aabb.min = -size/2.0f;
        bounds.wall->aabb.max = size/2.0f;
        
        glm::vec2 half_size = size/2.0f;
        bounds.wall->edges = {
            {{-half_size.x, -half_size.y}, {half_size.x, -half_size.y}, {0.0f, -1.0f}},
            {{half_size.x, -half_size.y}, {half_size.x, half_size.y}, {1.0f, 0.0f}},
            {{half_size.x, half_size.y}, {-half_size.x, half_size.y}, {0.0f, 1.0f}},
            {{-half_size.x, half_size.y}, {-half_size.x, -half_size.y}, {-1.0f, 0.0f}}
        };
        return bounds;
    }
    
    static CollisionBounds create_mesh(const std::vector<glm::vec2>& verts, float bound_radius) {
        CollisionBounds bounds;
        bounds.type = ColliderType::Mesh;
        bounds.mesh = new MeshCollider{verts, bound_radius};
        return bounds;
    }
};

struct Collision {
    Entity other;
    Collision(Entity &other) { this->other = other; }
};