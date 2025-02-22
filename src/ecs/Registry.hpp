#pragma once

#include <ecs/ComponentContainer.hpp>
#include <components/Components.hpp>
#include <ecs/IComponentContainer.hpp>
#include <optional>

#include <iostream>

// used to store the state of inputs for per_frame updates
struct InputState {
	bool w_down = false;
	bool a_down = false;
	bool s_down = false;
	bool d_down = false;
	glm::vec2 mouse_pos = {0.f, 0.f};
};

struct NearInteractable {
	bool is_active = false;
	Entity interactable;
	std::string message;
};

struct LockedTarget {
	bool is_active = false;
	Entity target;
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
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<AttackCooldown> attack_cooldowns;
	ComponentContainer<Team> teams;
	ComponentContainer<MoveWith> move_withs;
	ComponentContainer<RotateWith> rotate_withs;
	ComponentContainer<TextureName> textures;
	ComponentContainer<CollisionBounds> collision_bounds;
	ComponentContainer<InDodge> in_dodges;
	ComponentContainer<AIComponent> ais;
	ComponentContainer<BossAI> boss_ais;
	ComponentContainer<NearPlayer> near_players;
	ComponentContainer<NearCamera> near_cameras;
	ComponentContainer<Wall> walls;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<StaticObject> static_objects;
	ComponentContainer<StaggerCooldown> stagger_cooldowns;
	ComponentContainer<DeathCooldown> death_cooldowns;
	ComponentContainer<EnergyNoRegenCooldown> energy_no_regen_cooldowns;
	ComponentContainer<VisionToPlayer> vision_to_players;
	ComponentContainer<ProjectileModels> projectile_models;
	ComponentContainer<LightSource> light_sources;
	ComponentContainer<Interactable> interactables;
	ComponentContainer<Estus> estus;
	ComponentContainer<InRest> in_rests;
	ComponentContainer<AttackBuildup> buildups;
	GridMap grid_map;
	Entity player;
	Inventory inventory;
	NearInteractable near_interactable;
	LockedTarget locked_target;
	InputState input_state;
	glm::vec2 camera_pos;

	Registry() {
		m_registry_list.push_back(&motions);
		m_registry_list.push_back(&collisions);
		m_registry_list.push_back(&attackers);
		m_registry_list.push_back(&locomotion_stats);
		m_registry_list.push_back(&buffs);
		m_registry_list.push_back(&weapons);
		m_registry_list.push_back(&projectiles);
		m_registry_list.push_back(&attack_cooldowns);
		m_registry_list.push_back(&teams);
		m_registry_list.push_back(&move_withs);
		m_registry_list.push_back(&rotate_withs);
		m_registry_list.push_back(&textures);
		m_registry_list.push_back(&collision_bounds);
		m_registry_list.push_back(&in_dodges);
		m_registry_list.push_back(&ais);
		m_registry_list.push_back(&boss_ais);
		m_registry_list.push_back(&near_players);
		m_registry_list.push_back(&static_objects);
		m_registry_list.push_back(&walls);
		m_registry_list.push_back(&enemies);
		m_registry_list.push_back(&near_cameras);
		m_registry_list.push_back(&stagger_cooldowns);
		m_registry_list.push_back(&death_cooldowns);
		m_registry_list.push_back(&energy_no_regen_cooldowns);
		m_registry_list.push_back(&vision_to_players);
		m_registry_list.push_back(&projectile_models);
		m_registry_list.push_back(&light_sources);
		m_registry_list.push_back(&interactables);
		m_registry_list.push_back(&estus);
		m_registry_list.push_back(&in_rests);
		m_registry_list.push_back(&buildups);

		// create grid map entities
		grid_map = GridMap();
		for (int i = 0; i < int(Globals::update_distance) * 2; i++) {
			grid_map.grid_boxes.push_back(std::vector<GridMap::GridBox>());
			for (int j = 0; j < int(Globals::update_distance) * 2; j++) {
				grid_map.grid_boxes[i].push_back(GridMap::GridBox());
			}
		}
	}

	Registry& operator=(const Registry& other) {
		if (this != &other) {
			counter = other.counter;

			for (size_t i = 0; i < m_registry_list.size(); ++i) {
				*m_registry_list[i] = *other.m_registry_list[i];
			}

			grid_map = other.grid_map;
			player = other.player;
			inventory = other.inventory;
			near_interactable = other.near_interactable;
			input_state = other.input_state;
			camera_pos = other.camera_pos;

		}
		return *this;
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

	bool valid(Entity e) {
		for (IComponentContainer* reg : m_registry_list) {
			if (reg->has(e)) {
				return true;
			}
		}
		return false;
	}

	template<typename T>
	T* try_get_component(Entity e) {
		auto* container = get_container<T>();
		if (container && container->has(e)) {
			return &container->get(e);
		}
		return nullptr;
	}

	template<typename T>
	T& get_or_emplace_component(Entity e) {
		auto* container = get_container<T>();
		if (container) {
			if (container->has(e)) {
				return container->get(e);
			}
			return container->emplace(e);
		}
		throw std::runtime_error("Component type not registered");
	}

private:
	template<typename T>
	ComponentContainer<T>* get_container() {
		for (auto* container : m_registry_list) {
			if (auto* typed = dynamic_cast<ComponentContainer<T>*>(container)) {
				return typed;
			}
		}
		return nullptr;
	}
};