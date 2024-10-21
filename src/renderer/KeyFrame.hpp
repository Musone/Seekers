#pragma once

#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

class KeyFrame {
    float m_time_stamp;
    std::unordered_map<std::string, glm::mat4> pose;

public:
    KeyFrame(/* args */) {}
    ~KeyFrame() {}
    float get_time_stamp() const {
        return m_time_stamp;
    }
};