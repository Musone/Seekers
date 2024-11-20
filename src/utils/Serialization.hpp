#pragma once
#include <nlohmann/json.hpp>
#include <glm/vec2.hpp>
#include <stdexcept>

using json = nlohmann::json;

class SerializationError : public std::runtime_error {
public:
    SerializationError(const std::string& msg) : std::runtime_error(msg) {}
};

namespace Serialization {
    inline json serialize_vec2(const glm::vec2& v) {
        return {{"x", v.x}, {"y", v.y}};
    }
    
    inline void deserialize_vec2(glm::vec2& v, const json& j) {
        if (!j.contains("x") || !j.contains("y")) {
            throw SerializationError("Invalid vec2 format");
        }
        v.x = j["x"];
        v.y = j["y"];
    }

    inline json serialize_vec3(const glm::vec3& v) {
        return {{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }

    inline void deserialize_vec3(glm::vec3& v, const json& j) {
        if (!j.contains("x") || !j.contains("y") || !j.contains("z")) {
            throw SerializationError("Invalid vec3 format");
        }
        v.x = j["x"];
        v.y = j["y"];
        v.z = j["z"];
    }
}
