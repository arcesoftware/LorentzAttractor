# Lorenz 3D Explorer

A lightweight 3D visualization of the **Lorenz Attractor** (the "Butterfly Effect") built using C++ and SDL3. 

This project simulates a chaotic mathematical system and allows you to fly through it in real-time with a camera system similar to a 3D engine or Blender.

## Features
* **Live Generation:** The simulation runs continuously, adding new points to the "butterfly" every frame.
* **3D Flight:** Full WASD and Mouse controls to explore the attractor from any angle.
* **Dynamic Coloring:** Points shift color based on their position and the age of the trail.
* **Real-time Stats:** The window title displays current FPS and the total number of points being rendered.

## Controls
| Key | Action |
| :--- | :--- |
| **W / S** | Move Forward / Backward |
| **A / D** | Strafe Left / Right |
| **Space / L-Shift** | Move Up / Down |
| **Mouse** | Look around (Camera Pitch & Yaw) |
| **R** | Reset Camera to starting position |
| **ESC** | Toggle Mouse Lock (unlock to move window/use other apps) |

## How it Works
The code solves the Lorenz system of differential equations:
* $dx/dt = \sigma(y - x)$
* $dy/dt = x(\rho - z) - y$
* $dz/dt = xy - \beta z$

It then projects these 3D points onto your 2D screen using a perspective camera math, allowing you to "fly" through the data points.

## Requirements
* **C++17** or higher.
* **SDL3** (Simple DirectMedia Layer 3).
* **CMake** (for building).

## Quick Setup
1. Ensure SDL3 is installed on your system.
2. Update the paths in `CMakeLists.txt` to point to your SDL3 folders.
3. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
