#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

class Joint {
    unsigned int m_index;
    std::string m_name;
    std::vector<Joint> m_children;
    glm::mat4 m_animated_transform;
    glm::mat4 m_bind_transform;
    glm::mat4 m_inverse_bind_transform;

public:
    Joint(const int& index, const std::string& name, const glm::mat4& bind_transform) :
    m_index(index),
    m_name(name),
    m_bind_transform(bind_transform) {

    }

    ~Joint() {}

    void add_child(const Joint& joint) {
        m_children.push_back(joint);
    }

    glm::mat4 get_animated_transform() const {
        return m_animated_transform;
    }

    void set_animated_transform(const glm::mat4& animated_transform) {
        m_animated_transform = animated_transform;
    }

    glm::mat4 get_inverse_bind_transform() const {
        return m_inverse_bind_transform;
    }

    void set_inverse_bind_transform(const glm::mat4& parent_bind_transform) {
        m_inverse_bind_transform = glm::inverse(parent_bind_transform * m_bind_transform);
        for (Joint& c : m_children) {
            c.set_inverse_bind_transform(m_bind_transform);
        }
    }
};