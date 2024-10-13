#pragma once

#include <glm/glm.hpp>

#define PI 3.1415926535

namespace Transform {
    // Angle should be in radians!
    inline glm::mat4 create_rotation_matrix(const glm::vec3& angle) {
        const glm::mat4 rotate_z = {
            glm::cos(angle.z),  glm::sin(angle.z), 0, 0,
           -glm::sin(angle.z),  glm::cos(angle.z), 0, 0,
            0,                  0,                 1, 0,
            0,                  0,                 0, 1
        };
        const glm::mat4 rotate_y = {
            glm::cos(angle.y),  0, -glm::sin(angle.y), 0,
            0,                  1,  0,                 0,
            glm::sin(angle.y),  0,  glm::cos(angle.y), 0,
            0,                  0,  0,                 1
        };
        const glm::mat4 rotate_x = {
            1,  0,                   0,                  0,
            0,  glm::cos(angle.x),   glm::sin(angle.x),  0,
            0, -glm::sin(angle.x),   glm::cos(angle.x),  0,
            0,  0,                   0,                  1
        };
        return rotate_z * rotate_y * rotate_x;
    }

    inline glm::mat4 create_inverse_rotation_matrix(const glm::vec3& angle) {
        const glm::mat4 rotate_z = {
            glm::cos(-angle.z),  glm::sin(-angle.z), 0, 0,
           -glm::sin(-angle.z),  glm::cos(-angle.z), 0, 0,
            0,                          0,                         1, 0,
            0,                          0,                         0, 1
        };
        const glm::mat4 rotate_y = {
            glm::cos(-angle.y),  0, -glm::sin(-angle.y), 0,
            0,                          1,  0,                         0,
            glm::sin(-angle.y),  0,  glm::cos(-angle.y), 0,
            0,                          0,  0,                         1
        };

        const glm::mat4 rotate_x = {
            1,  0,                           0,                          0,
            0,  glm::cos(-angle.x),   glm::sin(-angle.x),  0,
            0, -glm::sin(-angle.x),   glm::cos(-angle.x),  0,
            0,  0,                           0,                          1
        };
        return rotate_x * rotate_y * rotate_z;
    }
    
    inline glm::mat4 create_translation_matrix(const glm::vec3& vector) {
        return {
            1,        0,        0,        0,
            0,        1,        0,        0,
            0,        0,        1,        0,
            vector.x, vector.y, vector.z, 1
        };
    }

    inline glm::mat4 create_scaling_matrix(const glm::vec3& scale) {
        return {
            scale.x, 0,       0,       0,
            0,       scale.y, 0,       0,
            0,       0,       scale.z, 0,
            0,       0,       0,       1
        };
    }

    inline glm::mat4 create_model_matrix(const glm::vec3& position, const glm::vec3& angle, const glm::vec3& scale) {
        return create_translation_matrix(position) * create_rotation_matrix(angle) * create_scaling_matrix(scale);
    }
};
