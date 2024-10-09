#pragma once

#include <ecs/ComponentContainer.hpp>
#include <components/Components.hpp>
#include <ecs/IComponentContainer.hpp>

#include <iostream>

class Registry {
	std::vector<IComponentContainer*> m_registry_list;

public:
	ComponentContainer<DeathTimer> death_timers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<ScreenState> screen_states;
	ComponentContainer<DebugComponent> debug_components;

	Registry() {
		m_registry_list.push_back(&death_timers);
		m_registry_list.push_back(&motions);
		m_registry_list.push_back(&collisions);
		m_registry_list.push_back(&players);
		m_registry_list.push_back(&screen_states);
		m_registry_list.push_back(&debug_components);
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