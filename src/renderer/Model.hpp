#pragma once

#include <glm/glm.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/Shader.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/Camera.hpp>
#include <utils/Transform.hpp>
// include joint and animator later.
#include <renderer/Joint.hpp>

class Model {
    Mesh* m_mesh;
    Texture2D* m_texture;
    Shader* m_shader;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    glm::mat4 m_model_matrix;
    // Root joint
    Joint* m_root_joint;
    unsigned int m_joint_count;
    glm::mat4* m_joint_transforms;
    // Animator
public:
    Model(Mesh* mesh, Texture2D* texture, Shader* shader, Joint* root_joint, int joint_count) :
    m_mesh(mesh),
    m_texture(texture),
    m_shader(shader),
    m_root_joint(root_joint),
    m_joint_count(joint_count),
    m_position(glm::vec3(0.0f)),
    m_rotation(glm::vec3(0.0f)),
    m_scale(glm::vec3(1.0f)),
    m_joint_transforms(nullptr) {
        if (m_joint_count > 0 && m_root_joint != nullptr) {
            m_root_joint->set_inverse_bind_transform(glm::mat4(1.0));
            m_joint_transforms = new glm::mat4[joint_count];
        }
        // create an animator
    }
    
    ~Model() {
        delete[] m_joint_transforms;
    }

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
        // seriously need a better way of handling uniforms...
        m_shader->set_uniform_3f("u_scale", m_scale);
        m_shader->set_uniform_mat4f("u_model", m_model_matrix);
    }
private:

};