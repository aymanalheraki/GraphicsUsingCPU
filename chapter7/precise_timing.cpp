//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

class PreciseTimer {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    
public:
    PreciseTimer() {
        reset();
    }
    
    void reset() {
        startTime = std::chrono::high_resolution_clock::now();
    }
    
    double getElapsedMs() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now - startTime;
        return std::chrono::duration<double, std::milli>(duration).count();
    }
    
    uint64_t getElapsedMsInt() {
        return static_cast<uint64_t>(getElapsedMs());
    }
};

class FrameRateController {
private:
    double targetFrameTime; // in milliseconds
    PreciseTimer frameTimer;
    
public:
    FrameRateController(double targetFPS) {
        targetFrameTime = 1000.0 / targetFPS;
    }
    
    void beginFrame() {
        frameTimer.reset();
    }
    
    void endFrame() {
        double elapsed = frameTimer.getElapsedMs();
        
        if (elapsed < targetFrameTime) {
            double sleepTime = targetFrameTime - elapsed;
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepTime));
        }
    }
    
    double getLastFrameTime() {
        return frameTimer.getElapsedMs();
    }
};

void drawFrame(SDL_Surface* surface, int frameNumber) {
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    
    // Clear to black
    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            pixels[y * pitch + x] = 0xFF000000;
        }
    }
    
    // Draw a simple moving bar to visualize timing
    int barWidth = 50;
    int barHeight = 20;
    int barX = (frameNumber * 2) % (surface->w - barWidth);
    int barY = surface->h / 2 - barHeight / 2;
    
    uint32_t barColor = 0xFF00FF00; // Green
    
    for (int y = barY; y < barY + barHeight && y < surface->h; ++y) {
        for (int x = barX; x < barX + barWidth && x < surface->w; ++x) {
            if (x >= 0 && y >= 0) {
                pixels[y * pitch + x] = barColor;
            }
        }
    }
    
    // Draw frame counter
    char frameText[32];
    sprintf(frameText, "Frame: %d", frameNumber);
    
    // Simple text rendering (draw frame number as bars)
    int textX = 10;
    int textY = 10;
    int digitWidth = 8;
    int digitHeight = 12;
    
    for (int i = 0; frameText[i] && textX + digitWidth < surface->w; ++i) {
        if (frameText[i] >= '0' && frameText[i] <= '9') {
            int digit = frameText[i] - '0';
            // Draw simple digit representation
            for (int dy = 0; dy < digitHeight; ++dy) {
                for (int dx = 0; dx < digitWidth; ++dx) {
                    // Simple pattern for digits (just vertical bars for simplicity)
                    if ((digit & (1 << (dx % 3))) && dy % 2 == 0) {
                        int px = textX + dx;
                        int py = textY + dy;
                        if (px < surface->w && py < surface->h) {
                            pixels[py * pitch + px] = 0xFFFFFFFF; // White
                        }
                    }
                }
            }
        }
        textX += digitWidth + 2;
    }
    
    SDL_UnlockSurface(surface);
}

int main(int argc, char** args) {
    
    bool quit = false;
    SDL_Surface* surface = NULL;
    SDL_Window* window = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return 1;
    }

    window = SDL_CreateWindow("Precise Timing Demo", 800, 600, SDL_WINDOW_RESIZABLE);

    if (!window) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        return 1;
    }

    surface = SDL_GetWindowSurface(window);

    if (!surface) {
        cout << "Error getting surface: " << SDL_GetError() << endl;
        return 1;
    }

    // Convert to ARGB8888 format
    surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);

    // Initialize timing
    FrameRateController frameController(60.0); // Target 60 FPS
    PreciseTimer totalTimer;
    
    int frameNumber = 0;
    double totalFrameTime = 0.0;
    int frameTimeCount = 0;
    
    cout << "Starting precise timing demo (60 FPS target)" << endl;
    cout << "Watch the moving green bar and frame counter" << endl;
    
    while (!quit) {
        frameController.beginFrame();
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        
        // Render frame
        drawFrame(surface, frameNumber);
        SDL_UpdateWindowSurface(window);
        
        frameController.endFrame();
        
        // Calculate frame time statistics
        double frameTime = frameController.getLastFrameTime();
        totalFrameTime += frameTime;
        frameTimeCount++;
        
        // Print statistics every 60 frames
        if (frameNumber % 60 == 0 && frameNumber > 0) {
            double avgFrameTime = totalFrameTime / frameTimeCount;
            double avgFPS = 1000.0 / avgFrameTime;
            
            cout << "Frame " << frameNumber 
                 << " - Avg Frame Time: " << avgFrameTime << "ms"
                 << " - Avg FPS: " << avgFPS << endl;
            
            totalFrameTime = 0.0;
            frameTimeCount = 0;
        }
        
        frameNumber++;
    }

    cout << "Total frames rendered: " << frameNumber << endl;
    cout << "Total time: " << totalTimer.getElapsedMs() / 1000.0 << " seconds" << endl;

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}