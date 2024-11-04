# Seekers

![Seekers Splash Screen](doc/Splashscreen.jpg)

## Team 23: Los Pollos Hermanos Gaming

Seekers is a top-down 2D/3D action RPG that melds dungeon crawling with open-world exploration. Set in the dark fantasy world of Eldoria, players take on the role of a Seeker, striving to uncover ancient secrets and restore balance to a land shrouded in darkness.

### Key Features

- **Dynamic Combat System**: Engage in skill-based, real-time battles using a variety of weapons and abilities
- **Procedural Generation**: Explore vast, mutated landscapes that change with each playthrough
- **3D Rendering**: Full 3D perspective with dynamic camera system
- **Advanced AI**: Enemies coordinate and adapt to player strategies
- **Rich Animation System**: Fluid character animations with skeletal rigging
- **Dynamic Lighting**: Real-time lighting with Blinn-Phong shading model

### Game Controls

- **Movement**: WASD keys
  - W: Move forward
  - A: Move left
  - S: Move backward
  - D: Move right
- **Camera Rotation**: Q and E keys
  - Q: Rotate camera left
  - E: Rotate camera right
- **Attack**: Left mouse button
- **Dodge**: Spacebar
- **Quit Game**: Z key
- **Aim**: Mouse movement

## Milestone 2 Implementation Details

### Mandatory Requirements (80%)

#### Improved Gameplay (60%)

1. **Game AI (15%)**
   - State-driven AI with decision trees
   - Implementation: 
      - `src/systems/AISystem.hpp`
      - `src/systems/GridMapSystem.hpp`
      - `src/components/AIComponents.hpp`
      - `src/utils/PathFinder.hpp`
   - Features: Patrol, chase, attack behaviors

2. **Animation (15%)**
   - Skeletal animation system
   - Implementation: 
      - `src/renderer/AnimatedModel.hpp`
      - `src/renderer/Animation.hpp`
      - `src/renderer/Animator.hpp`
      - `src/renderer/Skeleton.hpp`
   - Supports multiple animation states per character

3. **Assets (10%)**
   - Rich collection of 3D models and textures
   - Location: 
      - `src/models/`
      - `src/textures/`
   - Includes characters, weapons, environmental objects

4. **Mesh-based Collision Detection (10%)**
   - Complex collision detection system
   - Implementation: 
      - `src/systems/CollisionSystem.hpp`
      - `src/components/PhysicsComponent.hpp`
   - Handles entity-entity and entity-environment collisions

5. **Help/Tutorial (5%)**
   - Interactive tutorial system
   - Implementation: `src/systems/TutorialSystem.hpp`
   - Tutorial assets: `src/textures/tutorial/`

6. **FPS Counter (5%)**
   - Dynamic FPS display on window title
   - Implementation: `src/app/Application.hpp`
   - Always visible during gameplay

#### Playability & Stability (10%)
- Consistent frame rate management as shown by the FPS counter
- Optimized rendering pipeline: `src/renderer/Renderer.hpp`
- Stable collision detection: `src/systems/CollisionSystem.hpp`

#### Reporting (10%)
- Test plan: `/doc/test-plan.docx`
- Bug tracking: `/doc/bug-report.xlsx`
- Demo video: Submitted separately

### Creative Component (20%)

#### Graphics Features
1. **Advanced 3D Rendering**
   - Full 3D perspective camera: `src/renderer/Camera.hpp`
   - Dynamic view/projection matrices
   - Seamless 2D/3D switching

2. **Lighting System**
   - Blinn-Phong shading: `src/shaders/*BlinnPhong.fs.glsl`
   - Normal mapping
   - Dynamic shadows

3. **Animation System**
   - Skeletal animation: `src/renderer/Skeleton.hpp`
   - Joint hierarchy: `src/renderer/Joint.hpp`
   - Animation blending: `src/renderer/Animator.hpp`

#### Technical Features
1. **Scene Graph System**
   - Model hierarchy
   - Transform management: `src/utils/Transform.hpp`
   - Equipment attachment system

2. **Procedural Generation**
   - Dynamic map generation: `src/systems/ProceduralGenerationSystem.hpp`
   - Random encounter system
   - Environmental variety

3. **Advanced AI Behaviors**
   - Group coordination: `src/systems/AISystem.hpp`
   - Victory animations
   - Tactical positioning

### Additional Features
1. **Audio System**
   - Dynamic sound effects: `src/audio/`
   - Implementation: `src/systems/AudioSystem.hpp`
   - Contextual music system

2. **Path Finding**
   - Smart enemy navigation: `src/utils/PathFinder.hpp`
   - Obstacle avoidance
   - Dynamic path updates

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

### Acknowledgments

- OpenGL and GLFW communities
- SDL2 developers
- GLM developers
- Our dedicated team members and supportive instructors

---

<p align="center">
    <strong>Embark on an Epic Journey Through the Twisted Lands of Eldoria!</strong>
</p>
