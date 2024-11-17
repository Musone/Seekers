#pragma once

#include <ecs/Entity.hpp>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <typeindex>
#include <assert.h>

struct IComponentContainer
{
	virtual void clear() = 0;
	virtual size_t size() = 0;
	virtual void remove(Entity e) = 0;
	virtual bool has(Entity entity) = 0;
	virtual IComponentContainer& operator=(const IComponentContainer& other) = 0;
};


