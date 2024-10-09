#pragma once

#include <ecs/ComponentContainer.hpp>
#include <components/Components.hpp>
#include <ecs/IComponentContainer.hpp>

#include <iostream>

class Registry {
	std::vector<IComponentContainer*> m_registry_list;

public:
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Attacker> attackers;
	ComponentContainer<LocomotionStats> locomotion_stats;
	ComponentContainer<Buff> buffs;
	ComponentContainer<WeaponStats> weapon_stats;
	ComponentContainer<ProjectileStats> projectile_stats;
	ComponentContainer<AttackCooldown> attack_cooldowns;
	Entity player;
	
	Registry() {
		m_registry_list.push_back(&motions);
		m_registry_list.push_back(&collisions);
		m_registry_list.push_back(&attackers);
		m_registry_list.push_back(&locomotion_stats);
		m_registry_list.push_back(&buffs);
		m_registry_list.push_back(&weapon_stats);
		m_registry_list.push_back(&projectile_stats);
		m_registry_list.push_back(&attack_cooldowns);
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