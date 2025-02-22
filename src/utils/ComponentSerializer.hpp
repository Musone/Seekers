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
    
    // TextureName serialization
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

    // InDodge serialization
    inline json serialize_in_dodge(const InDodge& dodge) {
        return {
            {"source", Serialization::serialize_vec2(dodge.source)},
            {"destination", Serialization::serialize_vec2(dodge.destination)},
            {"origin_time", dodge.origin_time},
            {"duration", dodge.duration}
        };
    }
    
    inline void deserialize_in_dodge(InDodge& dodge, const json& j) {
        if (!j.contains("source") || !j.contains("destination") ||
            !j.contains("origin_time") || !j.contains("duration")) {
            throw SerializationError("Missing required fields in dodge data");
        }
        
        glm::vec2 source, destination;
        Serialization::deserialize_vec2(source, j["source"]);
        Serialization::deserialize_vec2(destination, j["destination"]);
        float origin_time = j["origin_time"];
        float duration = j["duration"];
        
        dodge = InDodge(source, destination, origin_time, duration);
    }

    // NearPlayer serialization (empty struct)
    inline json serialize_near_player(const NearPlayer&) {
        return json::object();
    }
    
    inline void deserialize_near_player(NearPlayer&, const json&) {
        // Nothing to deserialize
    }

    // NearCamera serialization (empty struct)
    inline json serialize_near_camera(const NearCamera&) {
        return json::object();
    }
    
    inline void deserialize_near_camera(NearCamera&, const json&) {
        // Nothing to deserialize
    }

    // VisionToPlayer serialization
    inline json serialize_vision_to_player(const VisionToPlayer& vision) {
        return {{"timer", vision.timer}};
    }
    
    inline void deserialize_vision_to_player(float& timer, const json& j) {
        if (!j.contains("timer")) {
            throw SerializationError("Missing timer in vision data");
        }
        timer = j["timer"];
    }

    // ProjectileModels serialization - we only save if models are present
    inline json serialize_projectile_models(const ProjectileModels& models) {
        return {
            {"has_arrow", models.arrow_model != nullptr},
            {"has_melee", models.melee_model != nullptr}
        };
    }

    inline void deserialize_projectile_models(ProjectileModels& models, const json& j) {
        // Models should be loaded/assigned elsewhere, we just track if they existed
        if (!j.contains("has_arrow") || !j.contains("has_melee")) {
            throw SerializationError("Missing fields in projectile models data");
        }
    }

    // Estus serialization
    inline json serialize_estus(const Estus& estus) {
        return {
            {"heal_amount", estus.heal_amount}
        };
    }

    inline void deserialize_estus(Estus& estus, const json& j) {
        if (!j.contains("heal_amount")) {
            throw SerializationError("Missing heal_amount in estus data");
        }
        estus.heal_amount = j["heal_amount"];
    }

    // Interactable serialization
    inline json serialize_interactable(const Interactable& interactable) {
        return {
            {"type", static_cast<int>(interactable.type)},
            {"entity_id", interactable.entity.get_id()},
            {"range", interactable.range}
        };
    }

    inline void deserialize_interactable(Interactable& interactable, const json& j, const Entity& mapped_entity) {
        if (!j.contains("type") || !j.contains("range")) {
            throw SerializationError("Missing fields in interactable data");
        }
        interactable.type = static_cast<INTERACTABLE_TYPE>(j["type"]);
        interactable.entity = mapped_entity;
        interactable.range = j["range"];
    }

    // LightSource serialization
    inline json serialize_light_source(const LightSource& light) {
        return {
            {"type", static_cast<int>(light.type)},
            {"brightness", light.brightness},
            {"pos", Serialization::serialize_vec3(light.pos)},
            {"colour", Serialization::serialize_vec3(light.colour)}
        };
    }

    inline void deserialize_light_source(LightSource& light, const json& j) {
        if (!j.contains("type") || !j.contains("brightness") || 
            !j.contains("pos") || !j.contains("colour")) {
            throw SerializationError("Missing fields in light source data");
        }
        light.type = static_cast<LIGHT_SOURCE_TYPE>(j["type"]);
        light.brightness = j["brightness"];
        Serialization::deserialize_vec3(light.pos, j["pos"]);
        Serialization::deserialize_vec3(light.colour, j["colour"]);
    }

    // Inventory serialization
    inline json serialize_inventory(const Inventory& inventory) {
        std::vector<unsigned int> estus_ids, weapon_ids;
        for (const auto& e : inventory.estus) estus_ids.push_back(e.get_id());
        for (const auto& w : inventory.weapons) weapon_ids.push_back(w.get_id());
        
        return {
            {"estus_ids", estus_ids},
            {"weapon_ids", weapon_ids},
            {"capacity", inventory.capacity}
        };
    }

    inline void deserialize_inventory(Inventory& inventory, const json& j, 
        const std::vector<std::pair<unsigned int, Entity>>& entity_map) {
        if (!j.contains("estus_ids") || !j.contains("weapon_ids") || !j.contains("capacity")) {
            throw SerializationError("Missing fields in inventory data");
        }
        
        inventory.capacity = j["capacity"];
        inventory.estus.clear();
        inventory.weapons.clear();
        
        for (const auto& id : j["estus_ids"]) {
            auto it = std::find_if(entity_map.begin(), entity_map.end(),
                [id](const auto& pair) { return pair.first == id; });
            if (it != entity_map.end()) {
                inventory.estus.push_back(it->second);
            }
        }
        
        for (const auto& id : j["weapon_ids"]) {
            auto it = std::find_if(entity_map.begin(), entity_map.end(),
                [id](const auto& pair) { return pair.first == id; });
            if (it != entity_map.end()) {
                inventory.weapons.push_back(it->second);
            }
        }
    }

    // NearInteractable serialization
    inline json serialize_near_interactable(const NearInteractable& near_or_something) {
        json data = {
            {"is_active", near_or_something.is_active},
            {"message", near_or_something.message}
        };
        if (near_or_something.is_active) {
            data["interactable_id"] = near_or_something.interactable.get_id();
        }
        return data;
    }

    inline void deserialize_near_interactable(NearInteractable& near_or_something, const json& j, 
        const std::vector<std::pair<unsigned int, Entity>>& entity_map) {
        if (!j.contains("is_active") || !j.contains("message")) {
            throw SerializationError("Missing fields in near_or_something interactable data");
        }
        
        near_or_something.is_active = j["is_active"];
        near_or_something.message = j["message"];
        
        if (near_or_something.is_active && j.contains("interactable_id")) {
            auto it = std::find_if(entity_map.begin(), entity_map.end(),
                [id = j["interactable_id"]](const auto& pair) { 
                    return pair.first == id; 
                });
            if (it != entity_map.end()) {
                near_or_something.interactable = it->second;
            }
        }
    }

    // LockedTarget serialization
    inline json serialize_locked_target(const LockedTarget& locked) {
        json data = {
            {"is_active", locked.is_active}
        };
        if (locked.is_active) {
            data["target_id"] = locked.target.get_id();
        }
        return data;
    }

    inline void deserialize_locked_target(LockedTarget& locked, const json& j, 
        const std::vector<std::pair<unsigned int, Entity>>& entity_map) {
        if (!j.contains("is_active")) {
            throw SerializationError("Missing fields in locked target data");
        }
        
        locked.is_active = j["is_active"];
        
        if (locked.is_active && j.contains("target_id")) {
            auto it = std::find_if(entity_map.begin(), entity_map.end(),
                [id = j["target_id"]](const auto& pair) { 
                    return pair.first == id; 
                });
            if (it != entity_map.end()) {
                locked.target = it->second;
            }
        }
    }
}