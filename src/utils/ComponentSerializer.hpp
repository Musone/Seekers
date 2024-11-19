#pragma once
#include "Serialization.hpp"
#include "components/Components.hpp"

namespace ComponentSerializer {
    // Motion serialization
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

    // LocomotionStats serialization
    inline json serialize_locomotion_stats(const LocomotionStats& stats) {
        return {
            {"health", stats.health},
            {"max_health", stats.max_health},
            {"energy", stats.energy},
            {"max_energy", stats.max_energy},
            {"poise", stats.poise},
            {"max_poise", stats.max_poise},
            {"defense", stats.defense},
            {"power", stats.power},
            {"agility", stats.agility},
            {"movement_speed", stats.movement_speed}
        };
    }
    
    inline void deserialize_locomotion_stats(LocomotionStats& stats, const json& j) {
        if (!j.contains("health") || !j.contains("max_health") ||
            !j.contains("energy") || !j.contains("max_energy") ||
            !j.contains("poise") || !j.contains("max_poise") ||
            !j.contains("defense") || !j.contains("power") ||
            !j.contains("agility") || !j.contains("movement_speed")) {
            throw SerializationError("Missing required fields in locomotion stats data");
        }
        
        stats.health = j["health"];
        stats.max_health = j["max_health"];
        stats.energy = j["energy"];
        stats.max_energy = j["max_energy"];
        stats.poise = j["poise"];
        stats.max_poise = j["max_poise"];
        stats.defense = j["defense"];
        stats.power = j["power"];
        stats.agility = j["agility"];
        stats.movement_speed = j["movement_speed"];
    }

    // TODO: Add serialization methods for other components:
    // - [X] Motion
    // - [X] LocomotionStats
    // - [ ] Weapon
    // - [ ] Team
    // - [ ] Wall/Enemy/StaticObject
    // - [ ] AIComponent
    // - [ ] Attacker
    // - [ ] TextureName
}