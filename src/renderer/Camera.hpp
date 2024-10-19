#pragma once

#include <utils/Transform.hpp>
#include <glm/glm.hpp>

class Camera {
    glm::vec3 m_position;
    glm::vec3 m_rotation;

    glm::mat4 m_view_project;
    glm::mat4 m_project;
    glm::mat4 m_view;

    glm::mat4 m_translate;
    glm::mat4 m_rotate;

    float m_fov;
    float m_width;
    float m_height;
    float m_far;
    float m_near;
public:
    Camera(const int& window_width, const int& window_height) : 
        m_position(0), 
        m_rotation(0), 
        m_translate(glm::mat4(1.0f)), 
        m_rotate(glm::mat4(1.0f)),
        m_fov(PI / 2),
        m_width(window_width),
        m_height(window_height),
        m_far(100),
        m_near(-1) {
        _update_view_matrix();
        _update_project_matrix();
    }
    
    ~Camera() {}

    glm::vec3 rotate_to_camera_direction(const glm::vec3& input) {
        const auto result = Transform::create_rotation_matrix(m_rotation) * glm::vec4({ input.x, input.y, input.z, 1 });
        return { result.x, result.y, result.z };
    }

    glm::mat4 get_view_project_matrix() const { return m_view_project; }

    glm::vec3 get_position() const { return m_position; }

    glm::vec3 get_rotation() const { return m_rotation; }

    glm::vec2 project_to_camera(const glm::vec3& world_position) const {
        glm::vec4 ndc = m_view_project * glm::vec4(world_position, 1);
        ndc = ndc / ndc.w; // perspective divide
        return { ndc.x, ndc.y };
    }

    void set_position(const glm::vec3& new_position) {
        m_position = new_position;
        m_translate = Transform::create_translation_matrix(-new_position);
        _update_view_matrix();
    }

    void set_rotation(const glm::vec3& new_rotation) {
        m_rotation = new_rotation;
        m_rotate = Transform::create_inverse_rotation_matrix(new_rotation);
        _update_view_matrix();
    }

private:
    void _update_project_matrix() {
        const float fov = glm::tan(m_fov / 2);
        m_project = {
            m_height / (m_width * fov), 0,      0,                                 0,
            0,                          1/fov,  0,                                 0,
            0,                          0,      -m_far / (m_far - m_near),         -1,
            0,                          0,       m_far * m_near / (m_far - m_near), 0
        };
        m_view_project = m_project * m_view;
    }

    void _update_view_matrix() {
        m_view = m_rotate * m_translate;
        m_view_project = m_project * m_view;
    }
};
