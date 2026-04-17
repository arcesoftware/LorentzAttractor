Anima3D / LorentzAttractor
=========================
Demo
----

![Demo GIF](demo.gif)

Overview
--------
This project visualizes several well-known 3D chaotic attractors and a "Unified Flow" Hamiltonian system using SDL3 for rendering. Implemented attractors (selectable with keys 1–5):

1. Lorenz
2. Aizawa
3. Rössler
4. Thomas
5. Unified Flow (Hamiltonian)

Build & Run
-----------
Requirements:
- CMake
- A C++ compiler (Visual Studio/MSVC on Windows)
- SDL3 development libraries available to the build/runtime

Commands (from repo root):

cmake -S . -B out -DCMAKE_BUILD_TYPE=Release
cmake --build out --config Release -- /m
out\Release\Lorenz3D.exe

Controls
--------
- Keys 1..5 : switch attractor (clears particles and resets state)
- Hold Left Ctrl : increase internal iteration steps (faster simulation)

Notes
-----
- If the program exits immediately, check lorenz_debug.log for startup diagnostics.
- The code writes lorenz_debug.log when SDL initialization fails.
- Consider using Git LFS for large binary files (.vs, build artefacts).

Source
------
All logic lives in main.cpp. Attractor parameters and behavior are implemented inline and can be tuned there.

License
-------
Add license information here if desired.

Contributing
------------
Open an issue or submit a PR. For large assets, use Git LFS.
