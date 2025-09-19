//Chapter 12: Advanced Case Studies - CPU-Only 2D Retro Game Engine
//Standard C++ libraries
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <array>
#include <memory>

//SDL3 for cross-platform display (software rendering only)
#include <SDL3/SDL.h>

using namespace std;
using namespace std::chrono;

// Book's software surface structure
struct SoftwareSurface {
    int width;
    int height;
    int pitch;
    uint32_t* pixels;
    
    SoftwareSurface(int w, int h) : width(w), height(h) {
        pitch = width * sizeof(uint32_t);
        pixels = new uint32_t[width * height];
        memset(pixels, 0, width * height * sizeof(uint32_t));
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

// RGB to ARGB conversion utility
uint32_t createColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Book's drawing primitives (CPU-only)
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

void blitSprite(SoftwareSurface* dst, SoftwareSurface* sprite, int x, int y, bool transparent = true, uint32_t transparentColor = 0xFF00FF) {
    for (int sy = 0; sy < sprite->height; ++sy) {
        for (int sx = 0; sx < sprite->width; ++sx) {
            uint32_t color = sprite->getPixel(sx, sy);
            
            if (!transparent || color != transparentColor) {
                dst->setPixel(x + sx, y + sy, color);
            }
        }
    }
}

// Book's tile-based background system
struct Tilemap {
    static const int MAP_WIDTH = 32;
    static const int MAP_HEIGHT = 24;
    static const int TILE_SIZE = 16;
    
    int tiles[MAP_HEIGHT][MAP_WIDTH];
    vector<unique_ptr<SoftwareSurface>> tileset;
    int scrollX, scrollY;
    
    Tilemap() : scrollX(0), scrollY(0) {
        // Create simple tileset
        for (int i = 0; i < 4; ++i) {
            auto tile = make_unique<SoftwareSurface>(TILE_SIZE, TILE_SIZE);
            
            // Create different colored tiles
            uint32_t colors[] = {
                createColor(64, 128, 64),   // Dark green
                createColor(128, 64, 64),   // Dark red
                createColor(64, 64, 128),   // Dark blue
                createColor(128, 128, 64)   // Dark yellow
            };
            
            tile->clear(colors[i]);
            
            // Add some pattern
            for (int y = 0; y < TILE_SIZE; ++y) {
                for (int x = 0; x < TILE_SIZE; ++x) {
                    if ((x + y) % 4 == 0) {
                        tile->setPixel(x, y, colors[i] | 0x404040);
                    }
                }
            }
            
            tileset.push_back(move(tile));
        }
        
        // Generate random tilemap
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 3);
        
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                tiles[y][x] = dis(gen);
            }
        }
    }
    
    void render(SoftwareSurface* target) {
        int startTileX = scrollX / TILE_SIZE;
        int startTileY = scrollY / TILE_SIZE;
        int offsetX = scrollX % TILE_SIZE;
        int offsetY = scrollY % TILE_SIZE;
        
        int tilesX = (target->width / TILE_SIZE) + 2;
        int tilesY = (target->height / TILE_SIZE) + 2;
        
        for (int ty = 0; ty < tilesY; ++ty) {
            for (int tx = 0; tx < tilesX; ++tx) {
                int mapX = (startTileX + tx) % MAP_WIDTH;
                int mapY = (startTileY + ty) % MAP_HEIGHT;
                
                int tileIndex = tiles[mapY][mapX];
                if (tileIndex >= 0 && tileIndex < tileset.size()) {
                    int screenX = tx * TILE_SIZE - offsetX;
                    int screenY = ty * TILE_SIZE - offsetY;
                    
                    blitSprite(target, tileset[tileIndex].get(), screenX, screenY, false);
                }
            }
        }
    }
    
    void scroll(int dx, int dy) {
        scrollX += dx;
        scrollY += dy;
        
        // Wrap around
        if (scrollX < 0) scrollX = 0;
        if (scrollY < 0) scrollY = 0;
        if (scrollX >= MAP_WIDTH * TILE_SIZE) scrollX = MAP_WIDTH * TILE_SIZE - 1;
        if (scrollY >= MAP_HEIGHT * TILE_SIZE) scrollY = MAP_HEIGHT * TILE_SIZE - 1;
    }
};

// Book's sprite system
struct Sprite {
    float x, y;
    float dx, dy;
    int width, height;
    uint32_t color;
    bool active;
    
    Sprite(float px, float py, int w, int h, uint32_t c) 
        : x(px), y(py), dx(0), dy(0), width(w), height(h), color(c), active(true) {}
    
    void update(float deltaTime) {
        if (!active) return;
        
        x += dx * deltaTime;
        y += dy * deltaTime;
    }
    
    void render(SoftwareSurface* surface) {
        if (!active) return;
        
        drawRect(surface, (int)x, (int)y, width, height, color);
    }
    
    bool collidesWith(const Sprite& other) const {
        return active && other.active &&
               x < other.x + other.width &&
               x + width > other.x &&
               y < other.y + other.height &&
               y + height > other.y;
    }
};

// Book's retro game engine
class RetroGameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    unique_ptr<SoftwareSurface> framebuffer;
    unique_ptr<Tilemap> tilemap;
    
    Sprite player;
    vector<Sprite> enemies;
    vector<Sprite> bullets;
    
    bool running;
    uint32_t lastTime;
    
    // Input state
    bool keys[512]; // Increased size for SDL3 keycodes
    
public:
    RetroGameEngine(int width, int height) 
        : window(nullptr), renderer(nullptr), texture(nullptr),
          player(width/2, height/2, 16, 16, createColor(255, 255, 0)), // Yellow player
          running(false), lastTime(0) {
        
        memset(keys, 0, sizeof(keys));
        
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw runtime_error("Failed to initialize SDL: " + string(SDL_GetError()));
        }
        
        window = SDL_CreateWindow("Chapter 12: CPU-Only Retro Game", width, height, 0);
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
        tilemap = make_unique<Tilemap>();
        
        // Initialize enemies
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> xDis(50, width - 50);
        uniform_int_distribution<> yDis(50, height - 50);
        
        for (int i = 0; i < 5; ++i) {
            enemies.emplace_back(xDis(gen), yDis(gen), 12, 12, createColor(255, 0, 0)); // Red enemies
            enemies.back().dx = (gen() % 200 - 100); // Random movement
            enemies.back().dy = (gen() % 200 - 100);
        }
        
        running = true;
        lastTime = SDL_GetTicks();
        
        cout << "Retro game engine initialized with CPU-only rendering" << endl;
    }
    
    ~RetroGameEngine() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    } else if (event.key.key < 512) {
                        keys[event.key.key] = true;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    if (event.key.key < 512) {
                        keys[event.key.key] = false;
                    }
                    break;
            }
        }
    }
    
    void updateGame(float deltaTime) {
        // Player movement
        const float speed = 200.0f;
        if (keys[SDLK_W] || keys[SDLK_UP]) player.y -= speed * deltaTime;
        if (keys[SDLK_S] || keys[SDLK_DOWN]) player.y += speed * deltaTime;
        if (keys[SDLK_A] || keys[SDLK_LEFT]) player.x -= speed * deltaTime;
        if (keys[SDLK_D] || keys[SDLK_RIGHT]) player.x += speed * deltaTime;
        
        // Keep player in bounds
        player.x = max(0.0f, min((float)(framebuffer->width - player.width), player.x));
        player.y = max(0.0f, min((float)(framebuffer->height - player.height), player.y));
        
        // Shooting
        static uint32_t lastShot = 0;
        if (keys[SDLK_SPACE] && SDL_GetTicks() - lastShot > 200) {
            bullets.emplace_back(player.x + player.width/2, player.y, 4, 8, createColor(255, 255, 255));
            bullets.back().dy = -400; // Move upward
            lastShot = SDL_GetTicks();
        }
        
        // Update enemies
        for (auto& enemy : enemies) {
            enemy.update(deltaTime);
            
            // Bounce off walls
            if (enemy.x <= 0 || enemy.x >= framebuffer->width - enemy.width) enemy.dx = -enemy.dx;
            if (enemy.y <= 0 || enemy.y >= framebuffer->height - enemy.height) enemy.dy = -enemy.dy;
            
            // Keep in bounds
            enemy.x = max(0.0f, min((float)(framebuffer->width - enemy.width), enemy.x));
            enemy.y = max(0.0f, min((float)(framebuffer->height - enemy.height), enemy.y));
        }
        
        // Update bullets
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->update(deltaTime);
            
            // Remove bullets that are off-screen
            if (it->y < 0 || it->y > framebuffer->height) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
        
        // Collision detection (CPU-based)
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
            bool hit = false;
            
            for (auto& enemy : enemies) {
                if (bulletIt->collidesWith(enemy)) {
                    enemy.active = false;
                    hit = true;
                    break;
                }
            }
            
            if (hit) {
                bulletIt = bullets.erase(bulletIt);
            } else {
                ++bulletIt;
            }
        }
        
        // Remove inactive enemies
        enemies.erase(remove_if(enemies.begin(), enemies.end(), 
                               [](const Sprite& s) { return !s.active; }), enemies.end());
        
        // Scroll tilemap based on player position
        static int lastPlayerX = 0, lastPlayerY = 0;
        int dx = (int)player.x - lastPlayerX;
        int dy = (int)player.y - lastPlayerY;
        if (abs(dx) > 5 || abs(dy) > 5) {
            tilemap->scroll(dx / 10, dy / 10);
            lastPlayerX = (int)player.x;
            lastPlayerY = (int)player.y;
        }
    }
    
    void render() {
        // Clear framebuffer (CPU operation)
        framebuffer->clear(createColor(32, 32, 64)); // Dark blue background
        
        // Render tilemap (CPU blitting)
        tilemap->render(framebuffer.get());
        
        // Render sprites (CPU drawing)
        player.render(framebuffer.get());
        
        for (auto& enemy : enemies) {
            enemy.render(framebuffer.get());
        }
        
        for (auto& bullet : bullets) {
            bullet.render(framebuffer.get());
        }
        
        // Draw UI
        drawRect(framebuffer.get(), 10, 10, 200, 20, createColor(0, 0, 0, 128)); // Semi-transparent background
        
        // Copy framebuffer to SDL texture (like GDI BitBlt)
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
        cout << "\n=== Retro Game Controls ===" << endl;
        cout << "WASD/Arrow Keys: Move player" << endl;
        cout << "SPACE: Shoot" << endl;
        cout << "ESC: Exit" << endl;
        cout << "Target: Destroy all red enemies!" << endl;
        
        while (running) {
            uint32_t currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;
            
            handleInput();
            updateGame(deltaTime);
            render();
            
            // Cap frame rate to ~60 FPS
            SDL_Delay(16);
            
            // Check win condition
            if (enemies.empty()) {
                cout << "\nCongratulations! All enemies destroyed!" << endl;
                SDL_Delay(2000);
                running = false;
            }
        }
    }
};

// Performance demonstration
void demonstratePerformance() {
    cout << "\n=== CPU Rendering Performance Demo ===" << endl;
    
    const int width = 800, height = 600;
    auto surface = make_unique<SoftwareSurface>(width, height);
    
    auto start = high_resolution_clock::now();
    
    // Simulate complex CPU rendering
    for (int frame = 0; frame < 100; ++frame) {
        surface->clear(createColor(0, 0, 0));
        
        // Draw many rectangles
        for (int i = 0; i < 1000; ++i) {
            int x = rand() % width;
            int y = rand() % height;
            int w = rand() % 50 + 10;
            int h = rand() % 50 + 10;
            uint32_t color = createColor(rand() % 256, rand() % 256, rand() % 256);
            drawRect(surface.get(), x, y, w, h, color);
        }
        
        // Draw many lines
        for (int i = 0; i < 500; ++i) {
            int x1 = rand() % width;
            int y1 = rand() % height;
            int x2 = rand() % width;
            int y2 = rand() % height;
            uint32_t color = createColor(rand() % 256, rand() % 256, rand() % 256);
            drawLine(surface.get(), x1, y1, x2, y2, color);
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "Rendered 100 complex frames in " << duration << " ms" << endl;
    cout << "Average frame time: " << (duration / 100.0) << " ms" << endl;
    cout << "Theoretical max FPS: " << (100000.0 / duration) << endl;
}

int main(int argc, char** args) {
    cout << "=== Chapter 12: Advanced Case Studies - CPU-Only Retro Game ===" << endl;
    cout << "Implementing complete 2D game engine without GPU acceleration" << endl;
    
    try {
        // Performance demonstration
        demonstratePerformance();
        
        cout << "\nStarting retro game..." << endl;
        
        // Run the actual game
        RetroGameEngine game(800, 600);
        game.run();
        
        cout << "\n=== CPU-Only Game Engine Summary ===" << endl;
        cout << "✓ Complete software rendering pipeline" << endl;
        cout << "✓ Tile-based background system" << endl;
        cout << "✓ Sprite animation and collision detection" << endl;
        cout << "✓ Real-time input handling" << endl;
        cout << "✓ Cross-platform compatibility" << endl;
        cout << "✓ Retro game aesthetics and performance" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}