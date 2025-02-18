# Seekers
Click the image below to view our Demo Video:

[![Seekers Demo](https://img.youtube.com/vi/hXh101_gjjw/0.jpg)](https://www.youtube.com/watch?v=hXh101_gjjw)

https://www.youtube.com/watch?v=hXh101_gjjw

## Team 23: Los Pollos Hermanos Gaming

Seekers is a third person 3D action RPG that melds dungeon crawling with open-world exploration. Set in the dark fantasy world of Eldoria, players take on the role of a Seeker, striving to uncover ancient secrets and restore balance to a land shrouded in darkness.

### Key Features

- **Open World & Dungeon System**: Explore a vast open world and venture into procedurally generated dungeons
- **Dynamic Combat System**: Engage in skill-based, real-time battles with enemy lock-on mechanics
- **Save/Load System**: Save your progress and continue your adventure later
- **Health Management**: Use potions to restore health and rest at bonfires
- **Rich Animation System**: Fluid character animations including sitting and combat moves
- **Dynamic Lighting**: Real-time lighting with environment-specific skyboxes

### Game Controls

- **Movement**: WASD keys
  - W: Move forward
  - A: Move left
  - S: Move backward
  - D: Move right
- **Camera Control**: Mouse Movement
- **Combat**:
  - Left Mouse Button: Attack
  - Right Mouse Button: Lock-on to enemy
  - Spacebar: Dodge
- **Interaction**:
  - F: Interact with objects (bonfire, portal)
  - 1: Use health potion
- **System**:
  - F5: Save game
  - F6: Load game
  - Z: Quit game

### Project Structure

- `src/`: Source code
  - `app/`: Core application logic (World, EntityFactory, InputManager)
  - `components/`: ECS components (AI, Combat, Physics, Render, etc.)
  - `ecs/`: Entity Component System implementation
  - `globals/`: Global variables and constants
  - `renderer/`: OpenGL rendering utilities
  - `systems/`: Game systems (Collision, Gameplay, Physics)
  - `shaders/`: GLSL shader files
  - `textures/`: Game textures and sprites
  - `utils/`: Utility functions and classes
  - `main.cpp`: Entry point of the application
- `doc/`: Documentation files
- `CMakeLists.txt`: CMake build configuration

## Milestone 3 Implementation Details

### Mandatory Requirements (60%)

#### Playability (15%) ✓

1. **Extended Gameplay**
   - Non-repetitive gameplay for 5+ minutes
   - Seamless world transitions
   - Dynamic enemy encounters
   - Progressive difficulty scaling

2. **Feature Integration**
   - Intuitive control scheme
   - Clear visual feedback
   - Smooth gameplay flow
   - Consistent mechanics

#### Robustness (15%) ✓

1. **Memory Management (5%)**
   - Efficient resource allocation
   - Proper cleanup of assets
   - Optimized asset loading
   - Memory usage monitoring

2. **User Input (5%)**
   - Robust input handling
   - Window management
   - Focus handling
   - Error recovery

3. **Performance (5%)**
   - Consistent frame rate
   - Optimized rendering
   - Efficient state management
   - Smooth transitions

#### Stability (20%) ✓

1. **Core Systems**
   - Stable save/load system
   - Reliable world transitions
   - Consistent combat mechanics
   - Proper resource management

2. **Technical Stability**
   - Cross-platform compatibility
   - Resolution independence
   - Graceful error handling
   - Crash prevention

#### Reporting (10%) ✓
- Comprehensive test plan
- Updated bug tracking
- Feature demonstration video

### Creative Component (40%)

#### Save/Load System (10%) ✓
- **Implementation**: `src/systems/SaveLoadSystem.hpp`
  - Complete game state preservation
  - Multiple save slots
  - Robust error handling
  - State verification

#### Complex Prescribed Motion (10%) ✓
- **Implementation**: `src/systems/AnimationSystem.hpp`
  - Bonfire rest animation
  - Smooth transition animations
  - Contextual movement sets
  - Blended animations

#### Environment System (20%) ✓

1. **Open World**
   - Vast explorable area
   - Interactive elements
   - Dynamic lighting
   - Atmospheric effects

2. **Dungeon System**
   - Procedural generation
   - Unique lighting system
   - Custom skybox themes
   - Light orb placement

3. **World Transitions**
   - Seamless loading
   - State preservation
   - Visual transitions
   - Audio adaptation

### Additional Features

1. **Enhanced Combat**
   - Enemy lock-on system
   - Tactical positioning
   - Visual targeting
   - Combat feedback

2. **Health System**
   - Health potion management
   - Bonfire restoration
   - Visual health feedback
   - Resource management

3. **Interaction System**
   - Context-sensitive prompts
   - Multiple interaction types
   - Clear feedback
   - State-based interactions

## Acknowledgments

- OpenGL and GLFW communities
- SDL2 developers
- GLM developers
- Our dedicated team members and supportive instructors

---

<p align="center">
    <strong>Embark on an Epic Journey Through the Twisted Lands of Eldoria!</strong>
</p>
