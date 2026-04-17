#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <iostream>

enum VisualMode { LORENZ, AIZAWA, ROSSER, THOMAS, UNIFIED_FLOW };

struct Vec3 { float x, y, z; };
struct Particle { Vec3 pos; SDL_Color color; };

// Constants
const float COUPLING = 0.5f;
const float ENERGY_LOSS = 0.999f;
const size_t MAX_PARTICLES = 100000;

int main(int argc, char* argv[]) {
    // 1. Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    int windowWidth = 1024, windowHeight = 768;
    SDL_Window* window = SDL_CreateWindow("Chaos Attractors 3D", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Camera and State
    float orbitYaw = 0.0f, orbitPitch = 0.0f, orbitDistance = 150.0f;
    VisualMode currentMode = LORENZ;
    float qx = 0.1f, qy = 0.1f, qz = 0.1f;
    float px = 0.0f, py = 0.0f, pz = 0.0f;
    const float dt = 0.01f;

    // Particle Ring Buffer
    std::vector<Particle> particles(MAX_PARTICLES);
    size_t particleCount = 0;
    size_t writeIdx = 0;

    bool running = true;
    SDL_Event event;

    while (running) {
        // 2. Event Handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                // Reset and Switch Modes
                if (event.key.key >= SDLK_1 && event.key.key <= SDLK_5) {
                    particleCount = 0;
                    writeIdx = 0;
                    qx = 0.1f; qy = 0.1f; qz = 0.1f;
                    px = py = pz = 0.0f;
                }
                if (event.key.key == SDLK_1) currentMode = LORENZ;
                if (event.key.key == SDLK_2) currentMode = AIZAWA;
                if (event.key.key == SDLK_3) currentMode = ROSSER;
                if (event.key.key == SDLK_4) currentMode = THOMAS;
                if (event.key.key == SDLK_5) currentMode = UNIFIED_FLOW;
            }
        }

        // 3. Physics Simulation
        const bool* state = SDL_GetKeyboardState(NULL);
        int steps = state[SDL_SCANCODE_LCTRL] ? 500 : 50;

        for (int i = 0; i < steps; i++) {
            float dx = 0, dy = 0, dz = 0;

            if (currentMode == LORENZ) {
                dx = 10.0f * (qy - qx);
                dy = qx * (28.0f - qz) - qy;
                dz = qx * qy - (8.0f / 3.0f) * qz;
            } else if (currentMode == AIZAWA) {
                dx = (qz - 0.7f) * qx - 3.5f * qy;
                dy = 3.5f * qx + (qz - 0.7f) * qy;
                dz = 0.6f + 0.95f * qz - (qz * qz * qz) / 3.0f - (qx * qx + qy * qy) * (1.0f + 0.25f * qz) + 0.1f * qz * qx * qx * qx;
            } else if (currentMode == ROSSER) {
                dx = -qy - qz;
                dy = qx + 0.2f * qy;
                dz = 0.2f + qz * (qx - 5.7f);
            } else if (currentMode == THOMAS) {
                dx = -0.208187f * qx + std::sin(qy);
                dy = -0.208187f * qy + std::sin(qz);
                dz = -0.208187f * qz + std::sin(qx);
            }

            if (currentMode != UNIFIED_FLOW) {
                qx += dx * dt;
                qy += dy * dt;
                qz += dz * dt;
            } else {
                float fx = -std::sin(qy) - COUPLING * qx;
                float fy = -std::sin(qz) - COUPLING * qy;
                float fz = -std::sin(qx) - COUPLING * qz;
                px = (px + fx * dt) * ENERGY_LOSS;
                py = (py + fy * dt) * ENERGY_LOSS;
                pz = (pz + fz * dt) * ENERGY_LOSS;
                qx += px * dt; qy += py * dt; qz += pz * dt;
            }

            // Store Particle
            float speed = std::sqrt(dx*dx + dy*dy + dz*dz);
            Particle& p = particles[writeIdx];
            p.pos = { qx, qy, qz };
            p.color = { (Uint8)SDL_clamp(50 + speed * 10, 0, 255), 150, 255, 255 };

            writeIdx = (writeIdx + 1) % MAX_PARTICLES;
            if (particleCount < MAX_PARTICLES) particleCount++;
        }

        // 4. Rendering
        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);
        SDL_RenderClear(renderer);

        orbitYaw += 0.005f; // Slow auto-rotation
        float cY = std::cos(orbitYaw), sY = std::sin(orbitYaw);

        for (size_t i = 0; i < particleCount; i++) {
            float vS = (currentMode == UNIFIED_FLOW) ? 50.0f : 2.5f;
            
            // Basic 3D to 2D projection
            float tx = particles[i].pos.x * vS;
            float ty = particles[i].pos.y * vS;
            float tz = particles[i].pos.z * vS;

            float rx = tx * cY - tz * sY;
            float rz = tx * sY + tz * cY + orbitDistance;

            if (rz > 1.0f) {
                float screenX = (rx * 800.0f / rz) + (windowWidth / 2.0f);
                float screenY = (ty * 800.0f / rz) + (windowHeight / 2.0f);

                SDL_SetRenderDrawColor(renderer, particles[i].color.r, particles[i].color.g, particles[i].color.b, 255);
                SDL_RenderPoint(renderer, screenX, screenY);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}