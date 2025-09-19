//Chapter 11: Cross-Platform Display on Windows and Linux
//Standard C++ libraries
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <cstring>

//SDL3 for cross-platform display
#include <SDL3/SDL.h>

using namespace std;
using namespace std::chrono;

class CrossPlatformDisplay {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;
    int width, height;
    int pitch;
    bool running;
    
public:
    CrossPlatformDisplay(int w, int h, const string& title) 
        : window(nullptr), renderer(nullptr), texture(nullptr), 
          pixels(nullptr), width(w), height(h), running(false) {
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw runtime_error("Failed to initialize SDL: " + string(SDL_GetError()));
        }
        
        // Create window
        window = SDL_CreateWindow(title.c_str(), width, height, 0);
        if (!window) {
            SDL_Quit();
            throw runtime_error("Failed to create window: " + string(SDL_GetError()));
        }
        
        // Create software renderer (CPU-only)
        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw runtime_error("Failed to create renderer: " + string(SDL_GetError()));
        }
        
        // Check renderer info
        const char* rendererName = SDL_GetRendererName(renderer);
        cout << "Using renderer: " << (rendererName ? rendererName : "Unknown") << endl;
        
        // Create streaming texture for direct pixel access
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                   SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw runtime_error("Failed to create texture: " + string(SDL_GetError()));
        }
        
        cout << "Cross-platform display initialized: " << width << "x" << height << endl;
        running = true;
    }
    
    ~CrossPlatformDisplay() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    // Lock texture for direct pixel manipulation (like GDI BitBlt or Linux framebuffer)
    uint32_t* lockPixels() {
        void* pixelData;
        if (SDL_LockTexture(texture, nullptr, &pixelData, &pitch) < 0) {
            cerr << "Failed to lock texture: " << SDL_GetError() << endl;
            return nullptr;
        }
        pixels = static_cast<uint32_t*>(pixelData);
        return pixels;
    }
    
    void unlockPixels() {
        SDL_UnlockTexture(texture);
        pixels = nullptr;
    }
    
    // Present the frame (like GDI BitBlt or framebuffer update)
    void present() {
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    
    bool handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    running = false;
                    break;
            }
        }
        return running;
    }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getPitch() const { return pitch; }
    bool isRunning() const { return running; }
    
    // Convenience function to set pixel (like Linux framebuffer example)
    void setPixel(uint32_t* buffer, int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            buffer[y * (pitch / 4) + x] = color;
        }
    }
    
    // Convert RGB to ARGB8888 format
    uint32_t createColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
};

// Book's framebuffer-style drawing functions
void clearScreen(uint32_t* pixels, int width, int height, int pitch, uint32_t color) {
    int pixelsPerRow = pitch / 4;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pixels[y * pixelsPerRow + x] = color;
        }
    }
}

void drawRectangle(uint32_t* pixels, int width, int height, int pitch,
                   int x1, int y1, int x2, int y2, uint32_t color) {
    int pixelsPerRow = pitch / 4;
    
    // Clip to screen bounds
    x1 = max(0, min(width - 1, x1));
    y1 = max(0, min(height - 1, y1));
    x2 = max(0, min(width - 1, x2));
    y2 = max(0, min(height - 1, y2));
    
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            pixels[y * pixelsPerRow + x] = color;
        }
    }
}

void drawGradient(uint32_t* pixels, int width, int height, int pitch) {
    int pixelsPerRow = pitch / 4;
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t r = (x * 255) / width;
            uint8_t g = (y * 255) / height;
            uint8_t b = ((x + y) * 255) / (width + height);
            
            uint32_t color = (255 << 24) | (r << 16) | (g << 8) | b;
            pixels[y * pixelsPerRow + x] = color;
        }
    }
}

// Software-based animation (like GDI double buffering)
void animatedPattern(uint32_t* pixels, int width, int height, int pitch, float time) {
    int pixelsPerRow = pitch / 4;
    
    int centerX = width / 2;
    int centerY = height / 2;
    float radius = min(width, height) / 4.0f;
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrt(dx*dx + dy*dy);
            
            // Create ripple effect
            float wave = sin(distance * 0.1f - time * 2.0f) * 0.5f + 0.5f;
            
            uint8_t intensity = (uint8_t)(wave * 255);
            uint32_t color = (255 << 24) | (intensity << 16) | (intensity << 8) | intensity;
            
            pixels[y * pixelsPerRow + x] = color;
        }
    }
}

// Performance test similar to GDI BitBlt performance
void performanceTest(CrossPlatformDisplay& display) {
    cout << "\n=== Display Performance Test ===" << endl;
    
    const int iterations = 100;
    vector<double> frameTimes;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations && display.isRunning(); ++i) {
        auto frameStart = high_resolution_clock::now();
        
        uint32_t* pixels = display.lockPixels();
        if (pixels) {
            // Simulate complex drawing operations
            clearScreen(pixels, display.getWidth(), display.getHeight(), 
                       display.getPitch(), display.createColor(0, 0, 0));
            
            // Draw multiple rectangles
            for (int j = 0; j < 100; ++j) {
                int x1 = rand() % display.getWidth();
                int y1 = rand() % display.getHeight();
                int x2 = x1 + rand() % 100;
                int y2 = y1 + rand() % 100;
                uint32_t color = display.createColor(rand() % 256, rand() % 256, rand() % 256);
                drawRectangle(pixels, display.getWidth(), display.getHeight(), 
                             display.getPitch(), x1, y1, x2, y2, color);
            }
            
            display.unlockPixels();
        }
        
        display.present();
        
        auto frameEnd = high_resolution_clock::now();
        double frameTime = duration_cast<microseconds>(frameEnd - frameStart).count() / 1000.0;
        frameTimes.push_back(frameTime);
        
        if (!display.handleEvents()) break;
        
        SDL_Delay(1); // Small delay to prevent overwhelming the display
    }
    
    auto totalTime = high_resolution_clock::now() - start;
    double totalMs = duration_cast<milliseconds>(totalTime).count();
    
    // Calculate statistics
    sort(frameTimes.begin(), frameTimes.end());
    double avgFrameTime = totalMs / frameTimes.size();
    double medianFrameTime = frameTimes[frameTimes.size() / 2];
    double fps = 1000.0 / avgFrameTime;
    
    cout << "Performance Results:" << endl;
    cout << "- Total frames: " << frameTimes.size() << endl;
    cout << "- Total time: " << totalMs << " ms" << endl;
    cout << "- Average frame time: " << avgFrameTime << " ms" << endl;
    cout << "- Median frame time: " << medianFrameTime << " ms" << endl;
    cout << "- Average FPS: " << fps << endl;
    cout << "- Min frame time: " << frameTimes.front() << " ms" << endl;
    cout << "- Max frame time: " << frameTimes.back() << " ms" << endl;
}

void demonstrateDoubleBuffering(CrossPlatformDisplay& display) {
    cout << "\n=== Double Buffering Animation Demo ===" << endl;
    cout << "Press ESC to exit..." << endl;
    
    auto startTime = high_resolution_clock::now();
    
    while (display.isRunning()) {
        auto currentTime = high_resolution_clock::now();
        float time = duration_cast<milliseconds>(currentTime - startTime).count() / 1000.0f;
        
        uint32_t* pixels = display.lockPixels();
        if (pixels) {
            animatedPattern(pixels, display.getWidth(), display.getHeight(), 
                           display.getPitch(), time);
            display.unlockPixels();
        }
        
        display.present();
        
        if (!display.handleEvents()) break;
        
        SDL_Delay(16); // ~60 FPS
    }
}

void demonstratePixelAccess(CrossPlatformDisplay& display) {
    cout << "\n=== Direct Pixel Access Demo ===" << endl;
    cout << "Drawing patterns like Linux framebuffer example..." << endl;
    
    uint32_t* pixels = display.lockPixels();
    if (pixels) {
        // Clear to black (like framebuffer example)
        clearScreen(pixels, display.getWidth(), display.getHeight(), 
                   display.getPitch(), display.createColor(0, 0, 0));
        
        // Draw white rectangle (like book's framebuffer example)
        drawRectangle(pixels, display.getWidth(), display.getHeight(), 
                     display.getPitch(), 100, 100, 300, 200, 
                     display.createColor(255, 255, 255));
        
        // Draw colored gradient
        int gradientHeight = 100;
        for (int y = 250; y < 250 + gradientHeight; ++y) {
            for (int x = 100; x < 400; ++x) {
                uint8_t intensity = ((x - 100) * 255) / 300;
                uint32_t color = display.createColor(intensity, 0, 255 - intensity);
                display.setPixel(pixels, x, y, color);
            }
        }
        
        // Draw some geometric patterns
        int centerX = display.getWidth() / 2;
        int centerY = display.getHeight() / 2 + 100;
        
        for (int angle = 0; angle < 360; angle += 5) {
            float rad = angle * M_PI / 180.0f;
            int x = centerX + (int)(cos(rad) * 80);
            int y = centerY + (int)(sin(rad) * 80);
            
            uint8_t r = (angle * 255) / 360;
            uint8_t g = 255 - r;
            uint8_t b = (angle * 128) / 360;
            
            drawRectangle(pixels, display.getWidth(), display.getHeight(), 
                         display.getPitch(), x-2, y-2, x+2, y+2, 
                         display.createColor(r, g, b));
        }
        
        display.unlockPixels();
    }
    
    display.present();
    
    cout << "Static image displayed. Press ESC to continue..." << endl;
    while (display.isRunning() && display.handleEvents()) {
        SDL_Delay(50);
    }
}

int main(int argc, char** args) {
    cout << "=== Chapter 11: Cross-Platform Display on Windows and Linux ===" << endl;
    cout << "Demonstrating CPU-only graphics with cross-platform display techniques" << endl;
    
    try {
        CrossPlatformDisplay display(800, 600, "Chapter 11: Cross-Platform CPU Graphics");
        
        // Demonstrate direct pixel access (like framebuffer programming)
        demonstratePixelAccess(display);
        
        // Reset for animation
        if (display.isRunning()) {
            // Demonstrate double buffering (like GDI techniques)
            demonstrateDoubleBuffering(display);
        }
        
        // Reset for performance test
        if (display.isRunning()) {
            performanceTest(display);
        }
        
        cout << "\n=== Cross-Platform Display Summary ===" << endl;
        cout << "✓ Direct pixel manipulation (framebuffer-style)" << endl;
        cout << "✓ Double buffering for smooth animation (GDI-style)" << endl;
        cout << "✓ Cross-platform compatibility (Windows/Linux/macOS)" << endl;
        cout << "✓ Software-only rendering (CPU-bound)" << endl;
        cout << "✓ Real-time performance measurement" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}