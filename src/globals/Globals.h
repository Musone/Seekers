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

#define MAP_WIDTH 150
#define MAP_HEIGHT 150
#define CAMERA_DISTANCE_FROM_WORLD 20
#include <utils/Timer.h>

namespace Globals {
    extern float cameraRotationSpeed;
    extern float dodgeMoveMag;
    extern float dodgeDuration;
    extern Timer timer;
}
