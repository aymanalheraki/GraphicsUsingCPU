//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

class DoubleBuffer {
private:
    SDL_Surface* frontBuffer;
    SDL_Surface* backBuffer;
    SDL_Window* window;
    
public:
    DoubleBuffer(SDL_Window* win) : window(win) {
        frontBuffer = SDL_GetWindowSurface(window);
        
        // Create back buffer with same format as front buffer
        backBuffer = SDL_CreateSurface(frontBuffer->w, frontBuffer->h, frontBuffer->format);
        
        if (!backBuffer) {
            cout << "Error creating back buffer: " << SDL_GetError() << endl;
        }
    }
    
    ~DoubleBuffer() {
        if (backBuffer) {
            SDL_DestroySurface(backBuffer);
        }
    }
    
    SDL_Surface* getBackBuffer() {
        return backBuffer;
    }
    
    void swap() {
        // Copy back buffer to front buffer
        SDL_BlitSurface(backBuffer, NULL, frontBuffer, NULL);
        SDL_UpdateWindowSurface(window);
    }
    
    void clear(uint32_t color = 0xFF000000) {
        SDL_LockSurface(backBuffer);
        uint32_t* pixels = (uint32_t*)backBuffer->pixels;
        int pitch = backBuffer->pitch / 4;
        
        for (int y = 0; y < backBuffer->h; ++y) {
            for (int x = 0; x < backBuffer->w; ++x) {
                pixels[y * pitch + x] = color;
            }
        }
        SDL_UnlockSurface(backBuffer);
    }
};

void drawCircle(SDL_Surface* surface, int cx, int cy, int radius, uint32_t color) {
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < surface->w && py >= 0 && py < surface->h) {
                    pixels[py * pitch + px] = color;
                }
            }
        }
    }
    SDL_UnlockSurface(surface);
}

void drawAnimatedScene(SDL_Surface* surface, double time) {
    // Draw multiple moving circles
    int centerX = surface->w / 2;
    int centerY = surface->h / 2;
    
    // Circle 1: orbiting
    int x1 = centerX + (int)(100 * cos(time));
    int y1 = centerY + (int)(100 * sin(time));
    drawCircle(surface, x1, y1, 20, 0xFFFF0000); // Red
    
    // Circle 2: orbiting in opposite direction
    int x2 = centerX + (int)(80 * cos(-time * 1.5));
    int y2 = centerY + (int)(80 * sin(-time * 1.5));
    drawCircle(surface, x2, y2, 15, 0xFF00FF00); // Green
    
    // Circle 3: vertical bounce
    int y3 = centerY + (int)(60 * sin(time * 2));
    drawCircle(surface, centerX + 150, y3, 12, 0xFF0000FF); // Blue
    
    // Circle 4: horizontal bounce
    int x4 = centerX + (int)(120 * sin(time * 1.2));
    drawCircle(surface, x4, centerY + 80, 18, 0xFFFFFF00); // Yellow
}

uint64_t getCurrentTimeMs() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int main(int argc, char** args) {
    
    bool quit = false;
    SDL_Window* window = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return 1;
    }

    window = SDL_CreateWindow("Double Buffering Demo", 800, 600, SDL_WINDOW_RESIZABLE);

    if (!window) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        return 1;
    }

    // Initialize double buffer
    DoubleBuffer doubleBuffer(window);
    
    uint64_t startTime = getCurrentTimeMs();
    uint64_t lastFrameTime = startTime;
    int frameCount = 0;
    
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        
        uint64_t currentTime = getCurrentTimeMs();
        double time = (currentTime - startTime) / 1000.0; // Time in seconds
        
        // Clear back buffer
        doubleBuffer.clear(0xFF000000); // Black background
        
        // Draw animated scene to back buffer
        drawAnimatedScene(doubleBuffer.getBackBuffer(), time);
        
        // Swap buffers (display the frame)
        doubleBuffer.swap();
        
        // Frame rate calculation and display
        frameCount++;
        if (currentTime - lastFrameTime >= 1000) {
            char title[256];
            sprintf(title, "Double Buffering Demo - FPS: %d", frameCount);
            SDL_SetWindowTitle(window, title);
            frameCount = 0;
            lastFrameTime = currentTime;
        }
        
        // Frame rate limiting to ~60 FPS
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}