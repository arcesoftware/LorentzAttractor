#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

// Simple 3D Vector
struct Vec3 {
    float x, y, z;
};

struct Particle {
    Vec3 pos;
    SDL_Color color;
};

// --- Initial Constants for Reset ---
const Vec3 START_POS = { 0.0f, 0.0f, -80.0f };
const float START_YAW = 0.0f;
const float START_PITCH = 0.0f;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    int windowWidth = 1024;
    int windowHeight = 768;
    SDL_Window* window = SDL_CreateWindow("Lorenz 3D Explorer", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Capture the mouse for 3D movement
    SDL_SetWindowRelativeMouseMode(window, true);

    // --- Camera State ---
    Vec3 camPos = START_POS;
    float camYaw = START_YAW;
    float camPitch = START_PITCH;

    float sensitivity = 0.002f;
    float speed = 40.0f;

    // --- Lorenz Simulation State ---
    float lx = 0.1f, ly = 0.0f, lz = 0.0f;
    const float sigma = 10.0f, rho = 28.0f, beta = 8.0f / 3.0f, dt = 0.005f;
    std::vector<Particle> particles;

    // --- Timing & FPS Variables ---
    bool running = true;
    SDL_Event event;
    Uint64 lastTime = SDL_GetTicksNS();
    float fpsTimer = 0.0f;
    int frameCount = 0;

    while (running) {
        // 1. Calculate Delta Time
        Uint64 currentTime = SDL_GetTicksNS();
        float deltaTime = (float)(currentTime - lastTime) / 1000000000.0f;
        lastTime = currentTime;

        if (deltaTime > 0.1f) deltaTime = 0.1f; // Cap delta to prevent physics glitches

        // 2. Update FPS counter in Title Bar every 0.25 seconds
        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 0.25f) {
            float fps = (float)frameCount / fpsTimer;
            std::string title = "Lorenz 3D | FPS: " + std::to_string((int)fps) + " | Points: " + std::to_string(particles.size());
            SDL_SetWindowTitle(window, title.c_str());
            fpsTimer = 0.0f;
            frameCount = 0;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
            }

            // Mouse Look
            if (event.type == SDL_EVENT_MOUSE_MOTION && SDL_GetWindowRelativeMouseMode(window)) {
                camYaw += event.motion.xrel * sensitivity;
                camPitch -= event.motion.yrel * sensitivity;
                camPitch = std::clamp(camPitch, -1.5f, 1.5f);
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                // Toggle mouse lock with Escape
                if (event.key.key == SDLK_ESCAPE) {
                    bool isLocked = SDL_GetWindowRelativeMouseMode(window);
                    if (isLocked) {
                        SDL_SetWindowRelativeMouseMode(window, false);
                        SDL_ShowCursor();
                    }
                    else {
                        SDL_SetWindowRelativeMouseMode(window, true);
                        SDL_HideCursor();
                    }
                }

                // R to Reset Camera
                if (event.key.scancode == SDL_SCANCODE_R) {
                    camPos = START_POS;
                    camYaw = START_YAW;
                    camPitch = START_PITCH;
                }
            }
        }

        // --- 3. Keyboard Movement (WASD) ---
        const bool* keys = SDL_GetKeyboardState(NULL);
        float s = speed * deltaTime;

        float sinY = std::sin(camYaw);
        float cosY = std::cos(camYaw);

        if (keys[SDL_SCANCODE_W]) { camPos.x += sinY * s; camPos.z += cosY * s; }
        if (keys[SDL_SCANCODE_S]) { camPos.x -= sinY * s; camPos.z -= cosY * s; }
        if (keys[SDL_SCANCODE_A]) { camPos.x -= cosY * s; camPos.z += sinY * s; }
        if (keys[SDL_SCANCODE_D]) { camPos.x += cosY * s; camPos.z -= sinY * s; }
        if (keys[SDL_SCANCODE_SPACE])  camPos.y += s;
        if (keys[SDL_SCANCODE_LSHIFT]) camPos.y -= s;

        // --- 4. Lorenz Generation ---
        for (int i = 0; i < 5; i++) {
            float dx = (sigma * (ly - lx)) * dt;
            float dy = (lx * (rho - lz) - ly) * dt;
            float dz = (lx * ly - beta * lz) * dt;
            lx += dx; ly += dy; lz += dz;

            Uint8 r = (Uint8)(127 * std::sin(0.0001 * particles.size()) + 128);
            Uint8 g = (Uint8)(127 * std::sin(0.0001 * particles.size() + 2) + 128);
            Uint8 b = (Uint8)(255);
            particles.push_back({ {lx, ly, lz}, {r, g, b, 255} });
        }

        if (particles.size() > 50000) particles.erase(particles.begin());

        // --- 5. Render ---
        SDL_SetRenderDrawColor(renderer, 5, 5, 15, 255);
        SDL_RenderClear(renderer);

        float cY = std::cos(-camYaw);
        float sY = std::sin(-camYaw);
        float cP = std::cos(-camPitch);
        float sP = std::sin(-camPitch);

        for (const auto& p : particles) {
            float tx = p.pos.x - camPos.x;
            float ty = p.pos.y - camPos.y;
            float tz = (p.pos.z - 25.0f) - camPos.z;

            float rx = tx * cY - tz * sY;
            float rz = tx * sY + tz * cY;
            float ry = ty * cP - rz * sP;
            float finalZ = ty * sP + rz * cP;

            if (finalZ > 1.0f) {
                float fov = 800.0f;
                float sx = (rx * fov / finalZ) + (windowWidth / 2.0f);
                float sy = (ry * fov / finalZ) + (windowHeight / 2.0f);

                if (sx >= 0 && sx < (float)windowWidth && sy >= 0 && sy < (float)windowHeight) {
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