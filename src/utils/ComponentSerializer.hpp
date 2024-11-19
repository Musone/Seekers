#pragma once
#include "Serialization.hpp"
#include "components/Components.hpp"

namespace ComponentSerializer {
    // Motion serialization only for initial testing
    inline json serialize_motion(const Motion& motion) {
        return {
            {"position", Serialization::serialize_vec2(motion.position)},
            {"angle", motion.angle},
            {"scale", Serialization::serialize_vec2(motion.scale)},
            {"velocity", Serialization::serialize_vec2(motion.velocity)},
            {"acceleration", Serialization::serialize_vec2(motion.acceleration)},
            {"rotation_velocity", motion.rotation_velocity},
            {"drag", motion.drag}
        };
    }
    
    inline void deserialize_motion(Motion& motion, const json& j) {
        if (!j.contains("position") || !j.contains("scale") || 
            !j.contains("velocity") || !j.contains("acceleration") ||
            !j.contains("angle") || !j.contains("rotation_velocity") ||
            !j.contains("drag")) {
            throw SerializationError("Missing required fields in motion data");
        }
        
        Serialization::deserialize_vec2(motion.position, j["position"]);
        Serialization::deserialize_vec2(motion.scale, j["scale"]);
        Serialization::deserialize_vec2(motion.velocity, j["velocity"]);
        Serialization::deserialize_vec2(motion.acceleration, j["acceleration"]);
        motion.angle = j["angle"];
        motion.rotation_velocity = j["rotation_velocity"];
        motion.drag = j["drag"];
    }

    // TODO: Add serialization methods for other components:
    // - Enemy
    // - Wall
    // - StaticObject
    // - Weapon
    // - Attacker
    // - AIComponent
    // - etc.
}
