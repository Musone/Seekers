#pragma once

#include <ecs/ComponentContainer.hpp>
#include <components/Components.hpp>
#include <ecs/IComponentContainer.hpp>

#include <iostream>

// used to store the state of inputs for per_frame updates
struct InputState {
	bool w_down = false;
	bool a_down = false;
	bool s_down = false;
	bool d_down = false;
	glm::vec2 mouse_pos = {0.f, 0.f};
};

class Registry {
	std::vector<IComponentContainer*> m_registry_list;

public:
	float counter = 0;

	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Attacker> attackers;
	ComponentContainer<LocomotionStats> locomotion_stats;
	ComponentContainer<Buff> buffs;
	ComponentContainer<Weapon> weapons;
	ComponentContainer<ProjectileStats> projectile_stats;
	ComponentContainer<AttackCooldown> attack_cooldowns;
	ComponentContainer<Team> teams;
	ComponentContainer<MoveWith> move_withs;
	ComponentContainer<RotateWith> rotate_withs;
	ComponentContainer<TextureName> textures;
	ComponentContainer<BoundingBox> bounding_boxes;
	ComponentContainer<InDodge> in_dodges;
	ComponentContainer<AIComponent> ais;
	ComponentContainer<NearPlayer> near_players;
	ComponentContainer<NearCamera> near_cameras;
	ComponentContainer<Wall> walls;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<StaticObject> static_objects;
	GridMap grid_map;
	Entity player;
	InputState input_state;
	glm::vec2 camera_pos;

	Registry() {
		m_registry_list.push_back(&motions);
		m_registry_list.push_back(&collisions);
		m_registry_list.push_back(&attackers);
		m_registry_list.push_back(&locomotion_stats);
		m_registry_list.push_back(&buffs);
		m_registry_list.push_back(&weapons);
		m_registry_list.push_back(&projectile_stats);
		m_registry_list.push_back(&attack_cooldowns);
		m_registry_list.push_back(&teams);
		m_registry_list.push_back(&move_withs);
		m_registry_list.push_back(&rotate_withs);
		m_registry_list.push_back(&textures);
		m_registry_list.push_back(&bounding_boxes);
		m_registry_list.push_back(&in_dodges);
		m_registry_list.push_back(&ais);
		m_registry_list.push_back(&near_players);
		m_registry_list.push_back(&static_objects);
		m_registry_list.push_back(&walls);
		m_registry_list.push_back(&enemies);
		m_registry_list.push_back(&near_cameras);
	}

	Registry(Registry const&) = delete;
	void operator=(Registry const&) = delete;

	static Registry& get_instance() {
		static Registry instance;
		return instance;
	}

	void hello() {
		std::cout << "hallo register\n"; 
	}

	void clear_all_components() {
		for (IComponentContainer* reg : m_registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (IComponentContainer* reg : m_registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (IComponentContainer* reg : m_registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (IComponentContainer* reg : m_registry_list)
			reg->remove(e);
	}
};