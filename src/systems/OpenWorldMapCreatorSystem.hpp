#pragma once

namespace OpenWorldMapCreatorSystem {
    inline void populate_open_world_map(Registry& registry) {
        // TODO: add everything here Sam
        EntityFactory::create_tree(registry, glm::vec2(0, 0)); //example
    }
};