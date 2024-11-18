//
// File that is used for keep all global variables used for tuning gameplay (such as cameraRotationSpeed)
//

#pragma once

#if __APPLE__
    #define WINDOW_WIDTH 1920 / 2
    #define WINDOW_HEIGHT 1280 / 2
#else
    #define WINDOW_WIDTH 1920
    #define WINDOW_HEIGHT 1280
#endif

#define MAP_WIDTH 500
#define MAP_HEIGHT 500
#define CAMERA_DISTANCE_FROM_WORLD 20.0f
#include <utils/Timer.h>

namespace Globals {
    extern float cameraRotationSpeed;
    extern float dodgeMoveMag;
    extern bool is_3d_mode;
    extern float dodgeDuration;
    extern Timer timer;
    extern float ai_distance_epsilon;
    extern float update_distance;
    extern float energy_regen_rate;
    extern float poise_regen_multiplier;
    extern float dodge_energy_cost;
    extern float energy_no_regen_duration;
    extern float static_render_distance;
    extern bool restart_renderer;
    extern float interactable_angle;
}
