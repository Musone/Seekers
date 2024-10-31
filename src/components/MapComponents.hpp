#pragma once

#include <vector>
#include <unordered_set>
#include <globals/Globals.h>

struct GridMap
{
    struct GridBox
    {
        bool is_occupied = false;
        int distance = -1;  // Initialize with -1 to indicate unvisited.
    };
    std::vector<std::vector<GridBox>> grid_boxes;
};

