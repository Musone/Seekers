#pragma once

#include <glm/glm.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/Shader.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/Camera.hpp>
#include <utils/Transform.hpp>
// include joint and animator later.

class Model {
    Mesh* m_mesh;
    Texture2D* m_texture;
    Shader* m_shader;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    glm::mat4 m_model_matrix;
    // Root joint
    // Animator
public:
    Model(Mesh* mesh, Texture2D* texture, Shader* shader) :
    m_mesh(mesh),
    m_texture(texture),
    m_shader(shader),
    m_position(glm::vec3(0.0f)),
    m_rotation(glm::vec3(0.0f)),
    m_scale(glm::vec3(1.0f)) {}
    
    ~Model() {}

    glm::mat4 get_model_matrix() const { return m_model_matrix; }
    unsigned int get_face_count() const { return m_mesh->get_face_count(); }

    void update() {
        m_model_matrix = Transform::create_model_matrix(
            m_position,
            m_rotation,
            m_scale
        );
    }

    void bind() const {
        m_mesh->bind();
        if (m_texture != nullptr){
            m_shader->bind();
            m_shader->set_uniform_1i("u_texture", m_texture->bind(1));
        }
        m_shader->set_uniform_3f("u_scale", m_scale);
        m_shader->set_uniform_mat4f("u_model", m_model_matrix);
    }
private:

};