#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

// --- Data Structures ---
enum VisualMode { LORENZ, AIZAWA, SACRED };

struct Vec3 { float x, y, z; };
struct Particle { Vec3 pos; SDL_Color color; };

// --- Constants & Sacred Geometry ---
const float PI = 3.14159265f;
const float GOLDEN_RATIO = 1.6180339f;
const float GOLDEN_ANGLE = PI * (3.0f - std::sqrt(5.0f)); // ~137.5 degrees

// Lorenz & Aizawa Parameters
const float L_SIGMA = 10.0f, L_RHO = 28.0f, L_BETA = 8.0f / 3.0f;
const float A_A = 0.95f, A_B = 0.7f, A_C = 0.6f, A_D = 3.5f, A_E = 0.25f, A_F = 0.1f;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    int windowWidth = 1024, windowHeight = 768;
    SDL_Window* window = SDL_CreateWindow("Sacred Chaos | 1:Lorenz 2:Aizawa 3:Sacred | L-CTRL: Turbo", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // --- Camera State (Orbital) ---
    float orbitYaw = 0.0f, orbitPitch = 0.0f, orbitDistance = 100.0f;
    Vec3 panOffset = { 0, 0, 0 };

    // --- Simulation State ---
    VisualMode currentMode = LORENZ;
    float lx = 0.1f, ly = 0.0f, lz = 0.0f;
    const float dt = 0.008f;
    std::vector<Particle> particles;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;

            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                if (event.wheel.y > 0) orbitDistance *= 0.9f;
                else orbitDistance *= 1.1f;
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    orbitYaw += event.motion.xrel * 0.01f;
                    orbitPitch = std::clamp(orbitPitch + event.motion.yrel * 0.01f, -1.5f, 1.5f);
                }
                if (event.motion.state & SDL_BUTTON_RMASK) {
                    panOffset.x += event.motion.xrel * (orbitDistance * 0.001f);
                    panOffset.y += event.motion.yrel * (orbitDistance * 0.001f);
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_1) { currentMode = LORENZ; particles.clear(); lx = 0.1f; ly = 0.1f; lz = 0.1f; }
                if (event.key.key == SDLK_2) { currentMode = AIZAWA; particles.clear(); lx = 0.1f; ly = 0.0f; lz = 0.1f; }
                if (event.key.key == SDLK_3) { currentMode = SACRED; particles.clear(); lx = 0.5f; ly = 0.5f; lz = 0.5f; }
                if (event.key.scancode == SDL_SCANCODE_R) { orbitYaw = 0; orbitPitch = 0; orbitDistance = 100.0f; panOffset = { 0,0,0 }; }
            }
        }

        // --- Multi-Step Physics Update ---
        const bool* keys = SDL_GetKeyboardState(NULL);
        int stepsPerFrame = keys[SDL_SCANCODE_LCTRL] ? 200 : 20;

        for (int i = 0; i < stepsPerFrame; i++) {
            float dx, dy, dz;
            if (currentMode == LORENZ) {
                dx = (L_SIGMA * (ly - lx)) * dt;
                dy = (lx * (L_RHO - lz) - ly) * dt;
                dz = (lx * ly - L_BETA * lz) * dt;
            }
            else if (currentMode == AIZAWA) {
                dx = ((lz - A_B) * lx - A_D * ly) * dt;
                dy = (A_D * lx + (lz - A_B) * ly) * dt;
                dz = (A_C + A_A * lz - (std::pow(lz, 3) / 3.0f) - (std::pow(lx, 2) + std::pow(ly, 2)) * (1.0f + A_E * lz) + A_F * lz * std::pow(lx, 3)) * dt;
            }
            else {
                // --- THE SACRED ATTRACTOR (Fibonacci/Golden Ratio Hybrid) ---
                float angle = particles.size() * GOLDEN_ANGLE;
                float radius = std::sqrt(lx * lx + ly * ly);

                // Spiral divergence mixed with Aizawa-style oscillation
                dx = (std::cos(angle) * lz - (GOLDEN_RATIO * ly)) * dt;
                dy = (std::sin(angle) * lz + (GOLDEN_RATIO * lx)) * dt;
                // Logistic growth (Fibonacci base) mapped to Z-axis
                dz = (GOLDEN_RATIO * lz * (1.0f - (lz / 30.0f)) - (radius * 0.5f)) * dt;
            }

            lx += dx; ly += dy; lz += dz;

            // Sacred Coloring: Cycle based on the Golden Ratio to avoid repeating patterns
            float colorPhase = (float)particles.size() * (GOLDEN_RATIO - 1.0f);
            Uint8 r = (Uint8)(127 * std::sin(colorPhase) + 128);
            Uint8 g = (Uint8)(127 * std::sin(colorPhase + 2.0f) + 128);
            Uint8 b = (Uint8)(127 * std::sin(colorPhase + 4.0f) + 128);

            particles.push_back({ {lx, ly, lz}, {r, g, b, 255} });
        }

        if (particles.size() > 120000) particles.erase(particles.begin(), particles.begin() + 100);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 2, 2, 10, 255); // Deep space blue
        SDL_RenderClear(renderer);

        float cosY = std::cos(orbitYaw), sinY = std::sin(orbitYaw);
        float cosP = std::cos(orbitPitch), sinP = std::sin(orbitPitch);

        for (const auto& p : particles) {
            float zCenter = (currentMode == LORENZ) ? 25.0f : (currentMode == SACRED ? 15.0f : 0.0f);
            float visualScale = (currentMode == LORENZ) ? 1.0f : (currentMode == SACRED ? 8.0f : 25.0f);

            float wx = p.pos.x * visualScale;
            float wy = p.pos.y * visualScale;
            float wz = (p.pos.z - zCenter) * visualScale;

            // Rotation Logic
            float rx = wx * cosY - wz * sinY;
            float rz_t = wx * sinY + wz * cosY;
            float ry = wy * cosP - rz_t * sinP;
            float rz = wy * sinP + rz_t * cosP;

            float finalZ = rz + orbitDistance;
            if (finalZ > 1.0f) {
                float sx = (rx + panOffset.x) * 1000.0f / finalZ + (windowWidth / 2.0f);
                float sy = (ry + panOffset.y) * 1000.0f / finalZ + (windowHeight / 2.0f);

                if (sx >= 0 && sx < windowWidth && sy >= 0 && sy < windowHeight) {
                    SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 255);
                    SDL_RenderPoint(renderer, sx, sy);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
