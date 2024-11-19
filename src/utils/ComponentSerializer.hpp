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

    // Weapon serialization
    inline json serialize_weapon(const Weapon& weapon) {
        return {
            {"type", static_cast<int>(weapon.type)},
            {"damage", weapon.damage},
            {"range", weapon.range},
            {"proj_speed", weapon.proj_speed},
            {"attack_cooldown", weapon.attack_cooldown},
            {"stagger_duration", weapon.stagger_duration},
            {"poise_points", weapon.poise_points},
            {"attack_energy_cost", weapon.attack_energy_cost},
            {"projectile_type", static_cast<int>(weapon.projectile_type)},
            {"enchantment", static_cast<int>(weapon.enchantment)}
        };
    }

    inline void deserialize_weapon(Weapon& weapon, const json& j) {
        weapon.type = static_cast<WEAPON_TYPE>(j["type"]);
        weapon.damage = j["damage"];
        weapon.range = j["range"];
        weapon.proj_speed = j["proj_speed"];
        weapon.attack_cooldown = j["attack_cooldown"];
        weapon.stagger_duration = j["stagger_duration"];
        weapon.poise_points = j["poise_points"];
        weapon.attack_energy_cost = j["attack_energy_cost"];
        weapon.projectile_type = static_cast<PROJECTILE_TYPE>(j["projectile_type"]);
        weapon.enchantment = static_cast<ENCHANTMENT>(j["enchantment"]);
    }

    // Team serialization
    inline json serialize_team(const Team& team) {
        return {
            {"team_id", static_cast<int>(team.team_id)}
        };
    }

    inline void deserialize_team(Team& team, const json& j) {
        team.team_id = static_cast<TEAM_ID>(j["team_id"]);
    }

    // Attacker serialization
    inline json serialize_attacker(const Attacker& attacker) {
        return {
            {"aim", Serialization::serialize_vec2(attacker.aim)},
            {"weapon_id", attacker.weapon.get_id()}
        };
    }

    inline void deserialize_attacker(Registry& registry, Attacker& attacker, const json& j) {
        Serialization::deserialize_vec2(attacker.aim, j["aim"]);
        // Note: weapon entity needs to be handled separately during full deserialization
    }

    // CollisionBounds serialization
    inline json serialize_collision_bounds(const CollisionBounds& bounds) {
        json j = {
            {"type", static_cast<int>(bounds.type)}
        };

        switch (bounds.type) {
            case ColliderType::Circle:
                j["radius"] = bounds.circle.radius;
                break;
            case ColliderType::AABB:
                j["min"] = Serialization::serialize_vec2(bounds.aabb.min);
                j["max"] = Serialization::serialize_vec2(bounds.aabb.max);
                break;
            case ColliderType::Wall:
                {
                    // Serialize AABB
                    j["aabb_min"] = Serialization::serialize_vec2(bounds.wall->aabb.min);
                    j["aabb_max"] = Serialization::serialize_vec2(bounds.wall->aabb.max);
                    
                    // Serialize edges
                    json edges_array = json::array();
                    for (const auto& edge : bounds.wall->edges) {
                        json edge_obj = {
                            {"start", Serialization::serialize_vec2(edge.start)},
                            {"end", Serialization::serialize_vec2(edge.end)},
                            {"normal", Serialization::serialize_vec2(edge.normal)}
                        };
                        edges_array.push_back(edge_obj);
                    }
                    j["edges"] = edges_array;
                }
                break;
            case ColliderType::Mesh:
                {
                    j["bound_radius"] = bounds.mesh->bound_radius;
                    
                    // Serialize vertices
                    json vertices_array = json::array();
                    for (const auto& vertex : bounds.mesh->vertices) {
                        vertices_array.push_back(Serialization::serialize_vec2(vertex));
                    }
                    j["vertices"] = vertices_array;
                }
                break;
        }
        return j;
    }

    inline void deserialize_collision_bounds(CollisionBounds& bounds, const json& j) {
        bounds.type = static_cast<ColliderType>(j["type"]);
        
        switch (bounds.type) {
            case ColliderType::Circle:
                bounds.circle.radius = j["radius"];
                break;
            case ColliderType::AABB:
                Serialization::deserialize_vec2(bounds.aabb.min, j["min"]);
                Serialization::deserialize_vec2(bounds.aabb.max, j["max"]);
                break;
            case ColliderType::Wall:
                bounds.wall = new WallCollider();
                // Deserialize AABB
                Serialization::deserialize_vec2(bounds.wall->aabb.min, j["aabb_min"]);
                Serialization::deserialize_vec2(bounds.wall->aabb.max, j["aabb_max"]);
                
                // Deserialize edges
                if (j.contains("edges")) {
                    for (const auto& edge : j["edges"]) {
                        LineSegment segment;
                        Serialization::deserialize_vec2(segment.start, edge["start"]);
                        Serialization::deserialize_vec2(segment.end, edge["end"]);
                        Serialization::deserialize_vec2(segment.normal, edge["normal"]);
                        bounds.wall->edges.push_back(segment);
                    }
                }
                break;
            case ColliderType::Mesh:
                bounds.mesh = new MeshCollider();
                bounds.mesh->bound_radius = j["bound_radius"];
                
                // Deserialize vertices
                if (j.contains("vertices")) {
                    for (const auto& vertex : j["vertices"]) {
                        glm::vec2 v;
                        Serialization::deserialize_vec2(v, vertex);
                        bounds.mesh->vertices.push_back(v);
                    }
                }
                break;
        }
    }

    // Wall serialization
    inline json serialize_wall(const Wall& wall) {
        return {
            {"type", static_cast<int>(wall.type)}
        };
    }
    
    inline void deserialize_wall(Wall& wall, const json& j) {
        if (!j.contains("type")) {
            throw SerializationError("Missing type in wall data");
        }
        wall.type = static_cast<WALL_TYPE>(j["type"]);
    }
    
    // Enemy serialization
    inline json serialize_enemy(const Enemy& enemy) {
        return {
            {"type", static_cast<int>(enemy.type)}
        };
    }
    
    inline void deserialize_enemy(Enemy& enemy, const json& j) {
        if (!j.contains("type")) {
            throw SerializationError("Missing type in enemy data");
        }
        enemy.type = static_cast<ENEMY_TYPE>(j["type"]);
    }
    
    // StaticObject serialization
    inline json serialize_static_object(const StaticObject& obj) {
        return {
            {"type", static_cast<int>(obj.type)}
        };
    }
    
    inline void deserialize_static_object(StaticObject& obj, const json& j) {
        if (!j.contains("type")) {
            throw SerializationError("Missing type in static object data");
        }
        obj.type = static_cast<STATIC_OBJECT_TYPE>(j["type"]);
    }
    
    // AIComponent serialization
    inline json serialize_ai_component(const AIComponent& ai) {
        json patrol_points = json::array();
        for (const auto& point : ai.patrol_points) {
            patrol_points.push_back(Serialization::serialize_vec2(point));
        }
        
        return {
            {"current_state", static_cast<int>(ai.current_state)},
            {"target_position", Serialization::serialize_vec2(ai.target_position)},
            {"detection_radius", ai.detection_radius},
            {"patrol_points", patrol_points}
        };
    }
    
    inline void deserialize_ai_component(AIComponent& ai, const json& j) {
        if (!j.contains("current_state") || !j.contains("target_position") ||
            !j.contains("detection_radius") || !j.contains("patrol_points")) {
            throw SerializationError("Missing required fields in AI component data");
        }
        
        ai.current_state = static_cast<AI_STATE>(j["current_state"]);
        Serialization::deserialize_vec2(ai.target_position, j["target_position"]);
        ai.detection_radius = j["detection_radius"];
        
        ai.patrol_points.clear();
        for (const auto& point : j["patrol_points"]) {
            glm::vec2 patrol_point;
            Serialization::deserialize_vec2(patrol_point, point);
            ai.patrol_points.push_back(patrol_point);
        }
    }
    
    // TextureName serialization (assuming it's a string component)
    inline json serialize_texture_name(const TextureName& texture) {
        return {
            {"name", texture.name}
        };
    }
    
    inline void deserialize_texture_name(TextureName& texture, const json& j) {
        if (!j.contains("name")) {
            throw SerializationError("Missing name in texture data");
        }
        texture.name = j["name"];
    }

    // MoveWith serialization
    inline json serialize_move_with(const MoveWith& move_with) {
        return {
            {"following_entity_id", move_with.following_entity_id}
        };
    }
    
    inline void deserialize_move_with(MoveWith& move_with, const json& j) {
        if (!j.contains("following_entity_id")) {
            throw SerializationError("Missing following_entity_id in move_with data");
        }
        move_with.following_entity_id = j["following_entity_id"];
    }

    // RotateWith serialization
    inline json serialize_rotate_with(const RotateWith& rotate_with) {
        return {
            {"following_entity_id", rotate_with.following_entity_id}
        };
    }
    
    inline void deserialize_rotate_with(RotateWith& rotate_with, const json& j) {
        if (!j.contains("following_entity_id")) {
            throw SerializationError("Missing following_entity_id in rotate_with data");
        }
        rotate_with.following_entity_id = j["following_entity_id"];
    }

    // Buff serialization
    inline json serialize_buff(const Buff& buff) {
        return {
            {"remaining_time", buff.remaining_time},
            {"health", buff.health},
            {"energy", buff.energy},
            {"defense", buff.defense},
            {"power", buff.power},
            {"agility", buff.agility},
            {"movement_speed", buff.movement_speed},
            {"effect", static_cast<int>(buff.effect)}
        };
    }
    
    inline void deserialize_buff(Buff& buff, const json& j) {
        if (!j.contains("remaining_time") || !j.contains("health") || 
            !j.contains("energy") || !j.contains("defense") || 
            !j.contains("power") || !j.contains("agility") || 
            !j.contains("movement_speed") || !j.contains("effect")) {
            throw SerializationError("Missing required fields in buff data");
        }
        
        buff.remaining_time = j["remaining_time"];
        buff.health = j["health"];
        buff.energy = j["energy"];
        buff.defense = j["defense"];
        buff.power = j["power"];
        buff.agility = j["agility"];
        buff.movement_speed = j["movement_speed"];
        buff.effect = static_cast<BUFF_EFFECT>(j["effect"]);
    }

    // Cooldown components serialization
    inline json serialize_cooldown(float timer) {
        return {{"timer", timer}};
    }
    
    inline void deserialize_cooldown(float& timer, const json& j) {
        if (!j.contains("timer")) {
            throw SerializationError("Missing timer in cooldown data");
        }
        timer = j["timer"];
    }

    // Projectile serialization
    inline json serialize_projectile(const Projectile& proj) {
        return {
            {"damage", proj.damage},
            {"range_remaining", proj.range_remaining},
            {"stagger_duration", proj.stagger_duration},
            {"poise_points", proj.poise_points},
            {"hit_locos", proj.hit_locos},
            {"projectile_type", static_cast<int>(proj.projectile_type)},
            {"enchantment", static_cast<int>(proj.enchantment)}
        };
    }
    
    inline void deserialize_projectile(Projectile& proj, const json& j) {
        if (!j.contains("damage") || !j.contains("range_remaining") ||
            !j.contains("stagger_duration") || !j.contains("poise_points") ||
            !j.contains("hit_locos") || !j.contains("projectile_type") ||
            !j.contains("enchantment")) {
            throw SerializationError("Missing required fields in projectile data");
        }
        
        proj.damage = j["damage"];
        proj.range_remaining = j["range_remaining"];
        proj.stagger_duration = j["stagger_duration"];
        proj.poise_points = j["poise_points"];
        proj.hit_locos = j["hit_locos"].get_to(proj.hit_locos);;
        proj.projectile_type = static_cast<PROJECTILE_TYPE>(j["projectile_type"]);
        proj.enchantment = static_cast<ENCHANTMENT>(j["enchantment"]);
    }

    // TODO: Add serialization methods for other components:
    // - [ ] Collisions
    // - [ ] TextureName
    // - [ ] InDodge
    // - [ ] NearPlayer
    // - [ ] NearCamera
    // - [ ] VisionToPlayer
    // - [ ] ProjectileModels
}