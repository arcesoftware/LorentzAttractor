#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

enum VisualMode { LORENZ, AIZAWA, FIBONACCI_HYBRID, SACRED_CHRYSALIS };

struct Vec3 { float x, y, z; };
struct Particle { Vec3 pos; SDL_Color color; };

const float GOLDEN_RATIO = 1.6180339f;
const float GOLDEN_ANGLE = 2.3999632f; // Radians (~137.5 degrees)

// Standard Constants
const float L_SIGMA = 10.0f, L_RHO = 28.0f, L_BETA = 8.0f / 3.0f;
const float A_A = 0.95f, A_B = 0.7f, A_C = 0.6f, A_D = 3.5f;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    int windowWidth = 1024, windowHeight = 768;
    SDL_Window* window = SDL_CreateWindow("Chrysalis Explorer | Keys 1-4 | L-CTRL Turbo", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    float orbitYaw = 0.0f, orbitPitch = 0.0f, orbitDistance = 120.0f;
    Vec3 panOffset = { 0, 0, 0 };

    VisualMode currentMode = LORENZ;
    float lx = 0.1f, ly = 0.0f, lz = 0.1f;
    const float dt = 0.007f;
    std::vector<Particle> particles;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                orbitDistance *= (event.wheel.y > 0) ? 0.9f : 1.1f;
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    orbitYaw += event.motion.xrel * 0.008f;
                    orbitPitch = std::clamp(orbitPitch + event.motion.yrel * 0.008f, -1.5f, 1.5f);
                }
                if (event.motion.state & SDL_BUTTON_RMASK) {
                    panOffset.x += event.motion.xrel * (orbitDistance * 0.001f);
                    panOffset.y += event.motion.yrel * (orbitDistance * 0.001f);
                }
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_1) { currentMode = LORENZ; particles.clear(); lx = 0.1f; ly = 0.1f; lz = 0.1f; }
                if (event.key.key == SDLK_2) { currentMode = AIZAWA; particles.clear(); lx = 0.1f; ly = 0.0f; lz = 0.1f; }
                if (event.key.key == SDLK_3) { currentMode = FIBONACCI_HYBRID; particles.clear(); lx = 0.5f; ly = 0.5f; lz = 0.5f; }
                if (event.key.key == SDLK_4) { currentMode = SACRED_CHRYSALIS; particles.clear(); lx = 0.1f; ly = 0.1f; lz = 0.1f; }
                if (event.key.scancode == SDL_SCANCODE_R) { orbitYaw = 0; orbitPitch = 0; orbitDistance = 120.0f; panOffset = { 0,0,0 }; }
            }
        }

        const bool* keys = SDL_GetKeyboardState(NULL);
        int steps = keys[SDL_SCANCODE_LCTRL] ? 250 : 25;

        for (int i = 0; i < steps; i++) {
            float dx, dy, dz;
            if (currentMode == LORENZ) {
                dx = (L_SIGMA * (ly - lx)) * dt;
                dy = (lx * (L_RHO - lz) - ly) * dt;
                dz = (lx * ly - L_BETA * lz) * dt;
            }
            else if (currentMode == AIZAWA) {
                dx = ((lz - A_B) * lx - A_D * ly) * dt;
                dy = (A_D * lx + (lz - A_B) * ly) * dt;
                dz = (A_C + A_A * lz - (std::pow(lz, 3) / 3.0f) - (lx * lx + ly * ly) * (1.0f + 0.25f * lz) + 0.1f * lz * std::pow(lx, 3)) * dt;
            }
            else if (currentMode == FIBONACCI_HYBRID) {
                float angle = particles.size() * GOLDEN_ANGLE;
                dx = (std::cos(angle) * lz - (GOLDEN_RATIO * ly)) * dt;
                dy = (std::sin(angle) * lz + (GOLDEN_RATIO * lx)) * dt;
                dz = (GOLDEN_RATIO * lz * (1.0f - (lz / 30.0f)) - (std::sqrt(lx * lx + ly * ly) * 0.5f)) * dt;
            }
            else {
                // --- IMPROVISED SACRED CHRYSALIS ---
                float t_angle = (float)particles.size() * GOLDEN_ANGLE;
                dx = (std::sin(GOLDEN_RATIO * lz) * lx - ly * std::cos(t_angle)) * dt;
                dy = (std::cos(GOLDEN_RATIO * lz) * ly + lx * std::sin(t_angle)) * dt;
                dz = (0.5f * (GOLDEN_RATIO - lz) * (lx * lx + ly * ly) - 0.2f * lz) * dt;
            }

            lx += dx; ly += dy; lz += dz;

            float colorPhase = (float)particles.size() * 0.0005f;
            Uint8 r = (Uint8)(127 * std::sin(colorPhase * GOLDEN_RATIO) + 128);
            Uint8 g = (Uint8)(127 * std::sin(colorPhase * (GOLDEN_RATIO + 1)) + 128);
            Uint8 b = (Uint8)(127 * std::sin(colorPhase * (GOLDEN_RATIO + 2)) + 128);
            particles.push_back({ {lx, ly, lz}, {r, g, b, 255} });
        }

        if (particles.size() > 150000) particles.erase(particles.begin(), particles.begin() + 150);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 0, 2, 5, 255);
        SDL_RenderClear(renderer);

        float cY = std::cos(-orbitYaw), sY = std::sin(-orbitYaw);
        float cP = std::cos(-orbitPitch), sP = std::sin(-orbitPitch);

        for (const auto& p : particles) {
            float zC = (currentMode == LORENZ) ? 25.0f : (currentMode == AIZAWA ? 0.0f : 10.0f);
            float vS = (currentMode == LORENZ) ? 1.0f : (currentMode == AIZAWA ? 25.0f : 12.0f);

            float wx = p.pos.x * vS, wy = p.pos.y * vS, wz = (p.pos.z - zC) * vS;
            float rx = wx * cY - wz * sY;
            float rz_t = wx * sY + wz * cY;
            float ry = wy * cP - rz_t * sP;
            float rz = wy * sP + rz_t * cP + orbitDistance;

            if (rz > 1.0f) {
                float sx = (rx + panOffset.x) * 1100.0f / rz + (windowWidth / 2.0f);
                float sy = (ry + panOffset.y) * 1100.0f / rz + (windowHeight / 2.0f);
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
