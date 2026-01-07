# Brick Breaker Game

A classic brick breaker game implemented in C++ using OpenGL, GLFW, and GLEW. This project features a paddle-based game where players break bricks and earn points while managing their lives.

## Project Overview

This is an OpenGL-based implementation of the classic Brick Breaker arcade game. The game includes features such as:

- **Interactive Paddle Control**: Move the paddle left and right to bounce the ball
- **Brick Grid**: A dynamic grid of bricks (6 rows × 18 columns) that can be destroyed
- **Collision Detection**: Accurate collision detection between ball, paddle, bricks, and walls
- **Score System**: Earn points by breaking bricks
- **Lives System**: Start with 3 lives; lose one when the ball goes out of bounds
- **Power-ups**: Special items that enhance gameplay
- **Paddle Zones**: Three sections of the paddle (left, middle, right) that affect ball trajectory
- **Visual Enhancements**: Shader-based coloring, depth gradients, and contoured elements
- **Game States**: Title screen, active gameplay, game over, and win conditions

## Prerequisites

Before building and running this project, ensure you have the following installed:

### System Requirements
- **Visual Studio 2022** or later (v143 toolset)
- **Windows 10 or later**
- **.NET Framework** compatible C++ environment

### Required Libraries
The project depends on three main graphics libraries:

1. **GLEW (OpenGL Extension Wrangler Library)** - For OpenGL function loading
2. **GLFW** - For window creation and input handling
3. **GLUT (FreeGLUT)** - For utility functions

All required dependency files are included in the `Dependencies/` folder:
- Header files (`.h` files)
- Library files (`.lib` files)
- DLL files (`glew32.dll`, `freeglut.dll`)

## Project Structure

```
brick-breaker-0.1/
├── README.md                      # This file
├── instructions.txt                     # Original project notes
├── project.sln                    # Visual Studio solution file
├── project.vcxproj               # Visual Studio C++ project file
├── .gitignore                    # Git ignore rules
│
├── src/
│   └── bricks.cpp                # Main game source code (1304 lines)
│
└── Dependencies/
    ├── GLEW/                     # OpenGL Extension Wrangler
    │   ├── include/GL/
    │   │   ├── glew.h
    │   │   ├── eglew.h
    │   │   ├── glxew.h
    │   │   └── wglew.h
    │   └── lib/
    │
    ├── GLFW/                     # Graphics Library Framework
    │   ├── include/GLFW/
    │   │   ├── glfw3.h
    │   │   └── glfw3native.h
    │   └── lib-vc2022/
    │
    └── GLUT/                     # OpenGL Utility Toolkit (FreeGLUT)
        ├── include/GL/
        │   ├── glut.h
        │   ├── freeglut.h
        │   ├── freeglut_std.h
        │   └── freeglut_ext.h
        └── lib/
```

## Installation and Setup

### Step 1: Prerequisites
- Install **Visual Studio 2022** with C++ development tools
- Ensure you have the Windows SDK installed

### Step 2: Clone/Download the Project
```bash
# Navigate to your projects directory
cd CV-Projects
git clone <repository-url>
cd brick-breaker-0.1
```

### Step 3: Open in Visual Studio
1. Open `project.sln` in Visual Studio 2022
2. The project is pre-configured with dependency paths pointing to the local `Dependencies/` folder

### Step 4: Build the Project
1. Select your desired configuration:
   - **Debug|Win32** - For 32-bit debugging
   - **Release|Win32** - For 32-bit release build
   - **Debug|x64** - For 64-bit debugging
   - **Release|x64** - For 64-bit release build

2. Build the solution:
   - Go to **Build → Build Solution** (or press `Ctrl+Shift+B`)

### Step 5: Run the Game
1. After successful build, run the executable:
   - Press `F5` to start with debugging
   - Or press `Ctrl+F5` to run without debugging

## How to Play

### Controls
- **Left Arrow** or **A Key**: Move paddle left
- **Right Arrow** or **D Key**: Move paddle right
- **SPACE**: Start the game / Resume after pause
- **P Key**: Pause/Resume the game
- **R Key**: Restart the game (after game over or winning)
- **Q Key**: Quit the game

### Gameplay
1. **Start Screen**: A title page is displayed at game start
2. **Objective**: Break all bricks by bouncing the ball with the paddle
3. **Paddle Zones**: 
   - **Left Section** (magenta): Deflects ball to the left
   - **Middle Section** (navy): Straight deflection
   - **Right Section** (cyan): Deflects ball to the right
4. **Scoring**: Earn points for each brick destroyed
5. **Lives**: You start with 3 lives
   - Lose a life when the ball falls below the paddle
   - Position the paddle and ball reset after losing a life
6. **Power-ups**: Collect special items that appear when breaking certain bricks
7. **Winning**: Destroy all bricks to win the game
8. **Game Over**: Lose all lives to trigger game over

## Technical Details

### Game Configuration
- **Window Resolution**: 600×700 pixels
- **Brick Grid**: 6 rows × 18 columns
- **Ball Radius**: 5 pixels (with contour thickness of 3 pixels)
- **Paddle Length**: 64 pixels
- **Paddle Height**: 20 pixels
- **Ball Velocity**: 3.5 units per frame (both X and Y components)
- **Initial Lives**: 3

### Architecture Highlights
- **Object-Oriented Design**: Uses helper functions for cleaner code organization
  - `updateBall()` - Ball physics and movement
  - `drawBall()` - Ball rendering
  - `checkCollisions()` - Collision detection
  - `handleCollisions()` - Collision response
  
- **OpenGL Rendering**: 
  - Uses GLEW for modern OpenGL function access
  - Implements shaders for dynamic coloring
  - Applies depth gradients and contours for visual enhancement
  
- **Input Handling**: 
  - Keyboard input for paddle control
  - Game state management (paused, active, game over, won)

### Compilation Notes
- **Platform Toolset**: Visual C++ 2022 (v143)
- **C++ Standard**: Compatible with modern C++
- **Include Paths**: Pre-configured for the `Dependencies/` folder structure
- **Library Linking**: GLEW, GLFW, and GLUT libraries are linked during compilation
- **DLL Dependencies**: Requires `glew32.dll` and `freeglut.dll` at runtime

## Features Implemented

✅ **Core Gameplay**
- Paddle movement and control
- Ball physics and bouncing
- Brick destruction
- Collision detection and response

✅ **Advanced Features**
- Score tracking
- Lives system with reset mechanics
- Multiple paddle zones for directional control
- Power-up system
- Game state management (title, active, pause, game over, win)
- Hole in the paddle for challenging gameplay

✅ **Visual Enhancements**
- Shader-based coloring system
- Gradient effects for depth perception
- Element contouring
- Color-coded paddle sections
- Styled brick grid

## Known Limitations

- **No Textures**: Texture mapping was not implemented due to system compatibility issues. Visual appeal is maintained through shaders and gradients instead.

## Building from Command Line (Alternative)

For those who prefer command-line compilation:

```batch
# Navigate to project directory
cd brick-breaker-0.1

# Compile with Visual Studio cl.exe
cl.exe /I"Dependencies\GLEW\include" /I"Dependencies\GLFW\include" /I"Dependencies\GLUT\include" ^
        src\bricks.cpp ^
        /link "Dependencies\GLEW\lib\glew32.lib" ^
              "Dependencies\GLFW\lib-vc2022\glfw3.lib" ^
              "Dependencies\GLUT\lib\freeglut.lib"
```

## Student Information

**Project Created By:**
- Soumaia Bouhouia (ID: 261053234)
- Alex Andrianavalontsalama (ID: 260979679)

## Troubleshooting

### Issue: DLL Not Found
**Solution**: Ensure that `glew32.dll` and `freeglut.dll` from the `Dependencies/` folder are in the same directory as the compiled executable or in your system PATH.

### Issue: Compilation Fails with Missing Headers
**Solution**: Verify that the project properties include the correct paths to the `Dependencies/` folder:
- Tools → Options → VC++ Directories
- Add `Dependencies\GLEW\include`, `Dependencies\GLFW\include`, and `Dependencies\GLUT\include`

### Issue: Window Doesn't Open
**Solution**: Check that GLFW initialization is successful. Ensure your graphics drivers are up to date and support OpenGL.

## Future Enhancements

Potential improvements for future versions:
- Texture mapping and advanced graphics
- Sound effects and background music
- Difficulty levels and progressive brick patterns
- High score tracking and leaderboard
- Multiple ball modes and special items
- Network multiplayer support
- Cross-platform support (Mac, Linux)

## License

This project was created for educational purposes as part of a computer science coursework.

---

**For detailed original notes, see [instructions.txt](instructions.txt)**
