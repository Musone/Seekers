#pragma once

#include <string>
#include <renderer/StaticModel.hpp>

struct TextureName {
    std::string name;
};

struct ProjectileModels {
    StaticModel* arrow_model = nullptr;
    StaticModel* melee_model = nullptr;
};