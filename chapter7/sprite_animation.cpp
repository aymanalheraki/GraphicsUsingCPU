//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <chrono>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

struct Sprite {
    uint8_t* imageData;
    int width;
    int height;
    int x;
    int y;
    int frameIndex;
    int totalFrames;
    int frameWidth;
    int frameHeight;
    int frameDelayMs;
    uint64_t lastFrameTime;
};

struct Vec2 {
    float x, y;
};

struct MovingSprite {
    Sprite sprite;
    Vec2 velocity;
};

uint64_t getCurrentTimeMs() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void updateSpritePosition(Sprite& sprite, int dx, int dy) {
    sprite.x += dx;
    sprite.y += dy;
}

void update(MovingSprite& ms, float deltaTime) {
    ms.sprite.x += static_cast<int>(ms.velocity.x * deltaTime);
    ms.sprite.y += static_cast<int>(ms.velocity.y * deltaTime);
}

void updateAnimation(Sprite& sprite, uint64_t currentTimeMs) {
    if (currentTimeMs - sprite.lastFrameTime >= sprite.frameDelayMs) {
        sprite.frameIndex = (sprite.frameIndex + 1) % sprite.totalFrames;
        sprite.lastFrameTime = currentTimeMs;
    }
}

void drawSpriteFrame(SDL_Surface* surface, const Sprite& sprite) {
    SDL_LockSurface(surface);
    uint32_t* framebuffer = (uint32_t*)surface->pixels;
    int fbWidth = surface->w;
    int pitch = surface->pitch / 4;
    
    // Create a simple animated sprite (colored squares)
    for (int y = 0; y < sprite.frameHeight && sprite.y + y < surface->h; ++y) {
        for (int x = 0; x < sprite.frameWidth && sprite.x + x < surface->w; ++x) {
            // Cycle through colors based on frame
            uint8_t r = (sprite.frameIndex * 50) % 256;
            uint8_t g = ((sprite.frameIndex * 30) + 100) % 256;
            uint8_t b = ((sprite.frameIndex * 70) + 200) % 256;
            uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
            
            if (sprite.x + x >= 0 && sprite.y + y >= 0) {
                framebuffer[(sprite.y + y) * pitch + (sprite.x + x)] = color;
            }
        }
    }
    SDL_UnlockSurface(surface);
}

void clear_surface(SDL_Surface* surface) {
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    
    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            pixels[y * pitch + x] = 0xFF000000; // Black background
        }
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

    window = SDL_CreateWindow("Sprite Animation", 800, 600, SDL_WINDOW_RESIZABLE);

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

    // Initialize sprite
    MovingSprite movingSprite;
    movingSprite.sprite.x = 100;
    movingSprite.sprite.y = 100;
    movingSprite.sprite.frameIndex = 0;
    movingSprite.sprite.totalFrames = 8;
    movingSprite.sprite.frameWidth = 50;
    movingSprite.sprite.frameHeight = 50;
    movingSprite.sprite.frameDelayMs = 100;
    movingSprite.sprite.lastFrameTime = getCurrentTimeMs();
    movingSprite.velocity.x = 60.0f;  // pixels per second
    movingSprite.velocity.y = 40.0f;
    
    uint64_t lastTime = getCurrentTimeMs();
    
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        
        uint64_t currentTime = getCurrentTimeMs();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Update sprite animation
        updateAnimation(movingSprite.sprite, currentTime);
        
        // Update sprite position with bouncing
        update(movingSprite, deltaTime);
        
        // Bounce off edges
        if (movingSprite.sprite.x <= 0 || movingSprite.sprite.x + movingSprite.sprite.frameWidth >= surface->w) {
            movingSprite.velocity.x = -movingSprite.velocity.x;
        }
        if (movingSprite.sprite.y <= 0 || movingSprite.sprite.y + movingSprite.sprite.frameHeight >= surface->h) {
            movingSprite.velocity.y = -movingSprite.velocity.y;
        }
        
        // Clear and draw
        clear_surface(surface);
        drawSpriteFrame(surface, movingSprite.sprite);
        
        SDL_UpdateWindowSurface(window);
        
        // Frame rate limiting to ~60 FPS
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}