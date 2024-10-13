#pragma once

class Entity {
	unsigned int id;
	static unsigned int id_count;
public:
	Entity() {
		id = ++id_count;
	}

	unsigned int get_id() const { return id; }
    
    // this enables automatic casting to int
	operator unsigned int() { return id; }
};