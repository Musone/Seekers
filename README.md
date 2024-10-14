# Seekers

## Team 23: Los Pollos Hermanos Gaming

Seekers is a top-down 2D action RPG that melds dungeon crawling with open-world exploration. Set in the dark fantasy world of Eldoria, players take on the role of a Seeker, striving to uncover ancient secrets and restore balance to a land shrouded in darkness.

### Key Features

- **Procedurally Generated World**: Explore vast, mutated landscapes that change with each playthrough.
- **Dynamic Combat System**: Engage in skill-based, real-time battles using a variety of weapons and abilities.
- **Time Dilation**: Unique mechanic allowing players to slow down time for tactical advantage.
- **Dungeon Crawling**: Delve into procedurally generated dungeons for randomized loot and face hand-crafted spires with unique bosses.
- **Weather System**: Experience a world affected by dynamic weather and day/night cycles that impact gameplay.
- **Permadeath with Progression**: Upon death, control a new Seeker, retaining global buffs but losing items and levels.

### Technical Highlights

- Developed in C++ using OpenGL for rendering
- Custom Entity Component System (ECS) for efficient game object management
- Procedural generation for world, dungeons, and weather systems
- Advanced enemy AI with state machines and pathfinding
- Physics-based interactions for projectiles and environmental effects

### Game Controls

- **Movement**: WASD keys
  - W: Move forward
  - A: Move left
  - S: Move backward
  - D: Move right
- **Camera Rotation**: Q and E keys
  - Q: Rotate camera left
  - E: Rotate camera right
- **Attack (Shoot Projectile)**: Left mouse button
- **Dodge**: Spacebar
- **Toggle 3D Mode**: Z key
- **Aim**: Mouse movement (in 2D mode)

### Getting Started

#### Prerequisites

- CMake (version 3.10 or higher)
- C++ compiler with C++17 support
- OpenGL
- GLFW
- GLM

#### Building and Running

1. Clone the repository:
   ```
   git clone https://github.com/your-repo/seekers.git
   cd seekers
   ```

2. Create a build directory and navigate to it:
   ```
   mkdir build && cd build
   ```

3. Generate the build files with CMake:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   cmake --build .
   ```

5. Run the executable:
   ```
   ./Seekers
   ```

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
