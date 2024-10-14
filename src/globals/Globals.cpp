//
// File for the definition of global variable.
// The values here are placeholders. They should be initialized by loading values from a .txt file
//      when the game is initialized (Start New Game, or Load Game is clicked by the player)
//

#include "Globals.h"

namespace Globals {
    float cameraRotationSpeed = 2.5f;
    float dodgeMoveMag = 7.0f;
    bool is_3d_mode = true;
    float dodgeDuration = 0.3f;
    Timer timer = Timer();
}