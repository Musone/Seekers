#pragma once

class Entity {
	unsigned int id;
	static unsigned int id_count;
public:
	Entity() {
		id = ++id_count;
	}
    
    // this enables automatic casting to int
	operator unsigned int() { return id; }
};