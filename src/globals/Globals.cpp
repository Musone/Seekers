//
// File for the definition of global variable.
// The values here are placeholders. They should be initialized by loading values from a .txt file
//      when the game is initialized (Start New Game, or Load Game is clicked by the player)
//

#include "Globals.h"

namespace Globals {
    float cameraRotationSpeed = 2.5f;
    float dodgeMoveMag = 5.0f;
    bool is_3d_mode = true;
    float dodgeDuration = 0.3f;
    Timer timer = Timer();
    float ai_distance_epsilon = 0.2f;
    float update_distance = 70.0f;
    float energy_regen_rate = 10.0f;
    float poise_regen_multiplier = 0.1f;
    float dodge_energy_cost = 10.0f;
    float energy_no_regen_duration = 2.0f;
    bool restart_renderer = true;
    float static_render_distance = 200.0f;
    float interactable_angle = 3.1415926535 / 6.0f;
    float lock_target_range = 20.0f;
    bool is_getting_up = false;
    void* ptr_window = nullptr;
    bool show_loading_screen = false;
}