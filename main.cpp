#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

enum VisualMode { LORENZ, AIZAWA };

struct Vec3 { float x, y, z; };
struct Particle { Vec3 pos; SDL_Color color; };

// Attractor Constants
const float L_SIGMA = 10.0f, L_RHO = 28.0f, L_BETA = 8.0f / 3.0f;
const float A_A = 0.95f, A_B = 0.7f, A_C = 0.6f, A_D = 3.5f, A_E = 0.25f, A_F = 0.1f;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    int windowWidth = 1024, windowHeight = 768;
    SDL_Window* window = SDL_CreateWindow("Chaos Orbiter | Left: Rotate | Right: Pan | Wheel: Zoom", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // --- Camera State (Orbital) ---
    float orbitYaw = 0.0f;
    float orbitPitch = 0.0f;
    float orbitDistance = 100.0f;
    Vec3 panOffset = { 0, 0, 0 }; // For right-click panning

    VisualMode currentMode = LORENZ;
    float lx = 0.1f, ly = 0.0f, lz = 0.0f;
    const float dt = 0.01f;
    std::vector<Particle> particles;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;

            // ZOOM with Mouse Wheel
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                if (event.wheel.y > 0) orbitDistance *= 0.9f;
                else if (event.wheel.y < 0) orbitDistance *= 1.1f;
            }

            // ROTATE & PAN with Mouse Motion
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (event.motion.state & SDL_BUTTON_LMASK) { // Left Click: Rotate
                    orbitYaw += event.motion.xrel * 0.01f;
                    orbitPitch += event.motion.yrel * 0.01f;
                    orbitPitch = std::clamp(orbitPitch, -1.5f, 1.5f);
                }
                if (event.motion.state & SDL_BUTTON_RMASK) { // Right Click: Pan
                    panOffset.x += event.motion.xrel * (orbitDistance * 0.001f);
                    panOffset.y += event.motion.yrel * (orbitDistance * 0.001f);
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_1) { currentMode = LORENZ; particles.clear(); lx = 0.1f; ly = 0.0f; lz = 0.0f; }
                if (event.key.key == SDLK_2) { currentMode = AIZAWA; particles.clear(); lx = 0.1f; ly = 0.0f; lz = 0.1f; }
                if (event.key.scancode == SDL_SCANCODE_R) { orbitYaw = 0; orbitPitch = 0; orbitDistance = 100.0f; panOffset = { 0,0,0 }; }
            }
        }

        // --- Physics Logic ---
        const bool* keys = SDL_GetKeyboardState(NULL);
        int stepsPerFrame = keys[SDL_SCANCODE_LCTRL] ? 150 : 15;

        for (int i = 0; i < stepsPerFrame; i++) {
            float dx, dy, dz;
            if (currentMode == LORENZ) {
                dx = (L_SIGMA * (ly - lx)) * dt;
                dy = (lx * (L_RHO - lz) - ly) * dt;
                dz = (lx * ly - L_BETA * lz) * dt;
            }
            else {
                dx = ((lz - A_B) * lx - A_D * ly) * dt;
                dy = (A_D * lx + (lz - A_B) * ly) * dt;
                dz = (A_C + A_A * lz - (std::pow(lz, 3) / 3.0f) - (std::pow(lx, 2) + std::pow(ly, 2)) * (1.0f + A_E * lz) + A_F * lz * std::pow(lx, 3)) * dt;
            }
            lx += dx; ly += dy; lz += dz;

            Uint8 r = (Uint8)(127 * std::sin(0.0001 * particles.size()) + 128);
            Uint8 g = (Uint8)(127 * std::sin(0.0001 * particles.size() + 2) + 128);
            particles.push_back({ {lx, ly, lz}, {r, g, (Uint8)(currentMode == LORENZ ? 255 : 150), 255} });
        }

        if (particles.size() > 100000) particles.erase(particles.begin(), particles.begin() + 20);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 5, 5, 15, 255);
        SDL_RenderClear(renderer);

        float cosY = std::cos(orbitYaw), sinY = std::sin(orbitYaw);
        float cosP = std::cos(orbitPitch), sinP = std::sin(orbitPitch);

        for (const auto& p : particles) {
            float zCenter = (currentMode == LORENZ) ? 25.0f : 0.0f;
            float visualScale = (currentMode == LORENZ) ? 1.0f : 25.0f;

            // 1. World coordinate relative to attractor center
            float wx = p.pos.x * visualScale;
            float wy = p.pos.y * visualScale;
            float wz = (p.pos.z - zCenter) * visualScale;

            // 2. Rotate World around Center
            float rx = wx * cosY - wz * sinY;
            float rz_temp = wx * sinY + wz * cosY;
            float ry = wy * cosP - rz_temp * sinP;
            float rz = wy * sinP + rz_temp * cosP;

            // 3. Apply Zoom and Panning
            float finalZ = rz + orbitDistance;

            if (finalZ > 1.0f) {
                float fov = 1000.0f;
                float sx = (rx + panOffset.x) * fov / finalZ + (windowWidth / 2.0f);
                float sy = (ry + panOffset.y) * fov / finalZ + (windowHeight / 2.0f);

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
