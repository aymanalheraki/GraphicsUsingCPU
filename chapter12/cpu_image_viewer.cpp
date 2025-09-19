//Chapter 12: Advanced Case Studies - Simple CPU-Only Image Viewer & Graphics Demo
//Standard C++ libraries
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <memory>

//SDL3 for cross-platform display (software rendering only)
#include <SDL3/SDL.h>

using namespace std;
using namespace std::chrono;

// Book's software surface structure
struct SoftwareSurface {
    int width;
    int height;
    uint32_t* pixels;
    
    SoftwareSurface(int w, int h) : width(w), height(h) {
        pixels = new uint32_t[width * height];
        clear();
    }
    
    ~SoftwareSurface() {
        delete[] pixels;
    }
    
    void clear(uint32_t color = 0x00000000) {
        for (int i = 0; i < width * height; ++i) {
            pixels[i] = color;
        }
    }
    
    void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = color;
        }
    }
    
    uint32_t getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y * width + x];
        }
        return 0;
    }
};

// Utility functions
uint32_t createColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Book's CPU-only drawing primitives
void drawPixel(SoftwareSurface* surface, int x, int y, uint32_t color) {
    surface->setPixel(x, y, color);
}

void drawRect(SoftwareSurface* surface, int x, int y, int w, int h, uint32_t color) {
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            surface->setPixel(x + dx, y + dy, color);
        }
    }
}

void drawLine(SoftwareSurface* surface, int x1, int y1, int x2, int y2, uint32_t color) {
    // Bresenham's line algorithm (CPU implementation)
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        surface->setPixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void drawCircle(SoftwareSurface* surface, int centerX, int centerY, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x*x + y*y <= radius*radius) {
                surface->setPixel(centerX + x, centerY + y, color);
            }
        }
    }
}

// Book's blitting function (CPU-only sprite rendering)
void blitSprite(SoftwareSurface* dst, SoftwareSurface* sprite, int x, int y, 
                bool transparent = true, uint32_t transparentColor = 0xFF00FF) {
    for (int sy = 0; sy < sprite->height; ++sy) {
        for (int sx = 0; sx < sprite->width; ++sx) {
            uint32_t color = sprite->getPixel(sx, sy);
            
            if (!transparent || (color & 0x00FFFFFF) != (transparentColor & 0x00FFFFFF)) {
                dst->setPixel(x + sx, y + sy, color);
            }
        }
    }
}

// Create a procedural texture/sprite (CPU-generated)
unique_ptr<SoftwareSurface> createProceduralSprite(int size, uint32_t baseColor) {
    auto sprite = make_unique<SoftwareSurface>(size, size);
    
    uint8_t r = (baseColor >> 16) & 0xFF;
    uint8_t g = (baseColor >> 8) & 0xFF;
    uint8_t b = baseColor & 0xFF;
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            // Create a gradient/pattern
            float distance = sqrt((x - size/2.0f) * (x - size/2.0f) + (y - size/2.0f) * (y - size/2.0f));
            float intensity = 1.0f - (distance / (size/2.0f));
            intensity = max(0.0f, min(1.0f, intensity));
            
            uint8_t nr = (uint8_t)(r * intensity);
            uint8_t ng = (uint8_t)(g * intensity);
            uint8_t nb = (uint8_t)(b * intensity);
            
            sprite->setPixel(x, y, createColor(nr, ng, nb));
        }
    }
    
    return sprite;
}

// Book's image viewer functionality
class CPUImageViewer {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    unique_ptr<SoftwareSurface> framebuffer;
    unique_ptr<SoftwareSurface> currentImage;
    
    float zoom;
    int panX, panY;
    bool running;
    
public:
    CPUImageViewer(int width, int height) 
        : window(nullptr), renderer(nullptr), texture(nullptr),
          zoom(1.0f), panX(0), panY(0), running(false) {
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw runtime_error("Failed to initialize SDL: " + string(SDL_GetError()));
        }
        
        window = SDL_CreateWindow("Chapter 12: CPU-Only Image Viewer", width, height, 0);
        if (!window) {
            SDL_Quit();
            throw runtime_error("Failed to create window: " + string(SDL_GetError()));
        }
        
        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw runtime_error("Failed to create renderer: " + string(SDL_GetError()));
        }
        
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                   SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw runtime_error("Failed to create texture: " + string(SDL_GetError()));
        }
        
        framebuffer = make_unique<SoftwareSurface>(width, height);
        
        // Create a procedural test image
        createTestImage();
        
        running = true;
        cout << "CPU-only image viewer initialized" << endl;
    }
    
    ~CPUImageViewer() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void createTestImage() {
        currentImage = make_unique<SoftwareSurface>(400, 300);
        
        // Create a colorful test pattern
        for (int y = 0; y < 300; ++y) {
            for (int x = 0; x < 400; ++x) {
                uint8_t r = (x * 255) / 400;
                uint8_t g = (y * 255) / 300;
                uint8_t b = ((x + y) * 255) / 700;
                
                currentImage->setPixel(x, y, createColor(r, g, b));
            }
        }
        
        // Add some geometric shapes
        drawCircle(currentImage.get(), 100, 75, 30, createColor(255, 255, 255));
        drawRect(currentImage.get(), 150, 50, 60, 40, createColor(255, 0, 0));
        drawLine(currentImage.get(), 250, 50, 350, 150, createColor(0, 255, 0));
        
        // Add some sprites
        auto sprite1 = createProceduralSprite(32, createColor(255, 255, 0));
        auto sprite2 = createProceduralSprite(24, createColor(0, 255, 255));
        
        blitSprite(currentImage.get(), sprite1.get(), 50, 200);
        blitSprite(currentImage.get(), sprite2.get(), 300, 220);
    }
    
    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_EQUALS:
                        case SDLK_PLUS:
                            zoom *= 1.2f;
                            if (zoom > 5.0f) zoom = 5.0f;
                            break;
                        case SDLK_MINUS:
                            zoom /= 1.2f;
                            if (zoom < 0.1f) zoom = 0.1f;
                            break;
                        case SDLK_UP:
                            panY -= 10;
                            break;
                        case SDLK_DOWN:
                            panY += 10;
                            break;
                        case SDLK_LEFT:
                            panX -= 10;
                            break;
                        case SDLK_RIGHT:
                            panX += 10;
                            break;
                        case SDLK_R:
                            // Reset view
                            zoom = 1.0f;
                            panX = panY = 0;
                            break;
                    }
                    break;
            }
        }
    }
    
    void render() {
        // Clear framebuffer
        framebuffer->clear(createColor(64, 64, 64));
        
        if (!currentImage) return;
        
        // Calculate scaled image dimensions
        int scaledWidth = (int)(currentImage->width * zoom);
        int scaledHeight = (int)(currentImage->height * zoom);
        
        // Calculate image position (centered with pan offset)
        int imageX = (framebuffer->width - scaledWidth) / 2 + panX;
        int imageY = (framebuffer->height - scaledHeight) / 2 + panY;
        
        // Render image with zoom and pan (CPU-based scaling)
        for (int y = 0; y < framebuffer->height; ++y) {
            for (int x = 0; x < framebuffer->width; ++x) {
                int relativeX = x - imageX;
                int relativeY = y - imageY;
                
                if (relativeX >= 0 && relativeX < scaledWidth && 
                    relativeY >= 0 && relativeY < scaledHeight) {
                    
                    // Map back to original image coordinates
                    int srcX = (int)(relativeX / zoom);
                    int srcY = (int)(relativeY / zoom);
                    
                    if (srcX >= 0 && srcX < currentImage->width && 
                        srcY >= 0 && srcY < currentImage->height) {
                        
                        uint32_t color = currentImage->getPixel(srcX, srcY);
                        framebuffer->setPixel(x, y, color);
                    }
                }
            }
        }
        
        // Draw UI overlay
        drawRect(framebuffer.get(), 10, 10, 300, 60, createColor(0, 0, 0, 128));
        
        // Copy framebuffer to SDL texture
        void* pixels;
        int pitch;
        if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
            memcpy(pixels, framebuffer->pixels, framebuffer->width * framebuffer->height * sizeof(uint32_t));
            SDL_UnlockTexture(texture);
        }
        
        // Present to screen
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    
    void run() {
        cout << "\n=== Image Viewer Controls ===" << endl;
        cout << "+/- : Zoom in/out" << endl;
        cout << "Arrow Keys: Pan" << endl;
        cout << "R: Reset view" << endl;
        cout << "ESC: Exit" << endl;
        
        while (running) {
            handleInput();
            render();
            SDL_Delay(16); // ~60 FPS
        }
    }
};

// Performance demonstration functions
void demonstrateCPURenderingPerformance() {
    cout << "\n=== CPU Rendering Performance Analysis ===" << endl;
    
    const int width = 800, height = 600;
    auto surface = make_unique<SoftwareSurface>(width, height);
    
    // Test 1: Fill performance
    auto start = high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        surface->clear(createColor(rand() % 256, rand() % 256, rand() % 256));
    }
    auto fillTime = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    
    // Test 2: Rectangle drawing
    start = high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        int x = rand() % width;
        int y = rand() % height;
        int w = rand() % 100 + 10;
        int h = rand() % 100 + 10;
        uint32_t color = createColor(rand() % 256, rand() % 256, rand() % 256);
        drawRect(surface.get(), x, y, w, h, color);
    }
    auto rectTime = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    
    // Test 3: Line drawing
    start = high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        int x1 = rand() % width;
        int y1 = rand() % height;
        int x2 = rand() % width;
        int y2 = rand() % height;
        uint32_t color = createColor(rand() % 256, rand() % 256, rand() % 256);
        drawLine(surface.get(), x1, y1, x2, y2, color);
    }
    auto lineTime = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    
    // Test 4: Circle drawing
    start = high_resolution_clock::now();
    for (int i = 0; i < 500; ++i) {
        int x = rand() % width;
        int y = rand() % height;
        int r = rand() % 50 + 5;
        uint32_t color = createColor(rand() % 256, rand() % 256, rand() % 256);
        drawCircle(surface.get(), x, y, r, color);
    }
    auto circleTime = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    
    cout << "Performance Results (CPU-only rendering):" << endl;
    cout << "- 100 screen fills: " << fillTime << " μs (" << (fillTime/100.0) << " μs per fill)" << endl;
    cout << "- 1000 rectangles: " << rectTime << " μs (" << (rectTime/1000.0) << " μs per rect)" << endl;
    cout << "- 1000 lines: " << lineTime << " μs (" << (lineTime/1000.0) << " μs per line)" << endl;
    cout << "- 500 circles: " << circleTime << " μs (" << (circleTime/500.0) << " μs per circle)" << endl;
    
    double totalPixels = width * height * 100 + 1000 * 50 * 50 + 1000 * 100 + 500 * 50 * 50; // Rough estimate
    double totalTime = (fillTime + rectTime + lineTime + circleTime) / 1000000.0; // Convert to seconds
    cout << "- Estimated pixel throughput: " << (totalPixels / totalTime / 1000000.0) << " MP/s" << endl;
}

void demonstrateImageProcessing() {
    cout << "\n=== CPU-based Image Processing Demo ===" << endl;
    
    auto image = make_unique<SoftwareSurface>(256, 256);
    
    // Create test image
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            uint8_t r = x;
            uint8_t g = y;
            uint8_t b = (x + y) / 2;
            image->setPixel(x, y, createColor(r, g, b));
        }
    }
    
    auto start = high_resolution_clock::now();
    
    // Convert to grayscale (CPU processing)
    for (int y = 0; y < image->height; ++y) {
        for (int x = 0; x < image->width; ++x) {
            uint32_t color = image->getPixel(x, y);
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;
            
            uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8; // Standard luminance formula
            image->setPixel(x, y, createColor(gray, gray, gray));
        }
    }
    
    auto grayscaleTime = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
    
    cout << "Grayscale conversion: " << grayscaleTime << " μs" << endl;
    cout << "Processing rate: " << (256*256*1000000.0/grayscaleTime) << " pixels/second" << endl;
}

int main(int argc, char** args) {
    cout << "=== Chapter 12: Advanced Case Studies - CPU-Only Graphics Systems ===" << endl;
    cout << "Demonstrating complete software rendering without GPU acceleration" << endl;
    
    try {
        // Performance demonstrations
        demonstrateCPURenderingPerformance();
        demonstrateImageProcessing();
        
        cout << "\nStarting CPU-only image viewer..." << endl;
        
        // Run the image viewer
        CPUImageViewer viewer(800, 600);
        viewer.run();
        
        cout << "\n=== CPU-Only Graphics Summary ===" << endl;
        cout << "✓ Complete software rendering pipeline" << endl;
        cout << "✓ Image viewing with zoom and pan capabilities" << endl;
        cout << "✓ Procedural texture generation" << endl;
        cout << "✓ Sprite blitting with transparency" << endl;
        cout << "✓ Real-time image processing" << endl;
        cout << "✓ Cross-platform compatibility" << endl;
        cout << "✓ Performance measurement and optimization" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}