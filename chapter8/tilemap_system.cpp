//Chapter 8: Real-Time 2D Effects - Tilemap System
//Standard C++ libraries
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <random>

//SDL3 library
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

// Book's exact tilemap data structures
struct Tile {
    uint32_t* pixels;   // Pixel data (width x height)
    int width;
    int height;
};

struct TileMap {
    int rows;
    int cols;
    int tileWidth;
    int tileHeight;
    std::vector<uint16_t> mapData;   // Tile indices
    std::vector<Tile> tileSet;
};

struct Viewport {
    int xOffset;
    int yOffset;
    int width;
    int height;
};

// Helper function to create a colored tile
Tile createColorTile(int width, int height, uint32_t color) {
    Tile tile;
    tile.width = width;
    tile.height = height;
    tile.pixels = new uint32_t[width * height];
    
    for (int i = 0; i < width * height; ++i) {
        tile.pixels[i] = color;
    }
    
    return tile;
}

// Helper function to create a pattern tile
Tile createPatternTile(int width, int height, uint32_t color1, uint32_t color2) {
    Tile tile;
    tile.width = width;
    tile.height = height;
    tile.pixels = new uint32_t[width * height];
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Checkerboard pattern
            bool isColor1 = ((x / 4) + (y / 4)) % 2 == 0;
            tile.pixels[y * width + x] = isColor1 ? color1 : color2;
        }
    }
    
    return tile;
}

// Book's exact drawTileClipped implementation
void drawTileClipped(
    uint32_t* framebuffer, int fbWidth, int fbHeight,
    const Tile& tile, int destX, int destY
) {
    // Calculate clipping bounds
    int srcStartX = std::max(0, -destX);
    int srcStartY = std::max(0, -destY);
    int srcEndX = std::min(tile.width, fbWidth - destX);
    int srcEndY = std::min(tile.height, fbHeight - destY);
    
    // Skip if tile is completely outside framebuffer
    if (srcStartX >= srcEndX || srcStartY >= srcEndY) return;
    
    for (int y = srcStartY; y < srcEndY; ++y) {
        for (int x = srcStartX; x < srcEndX; ++x) {
            int fbX = destX + x;
            int fbY = destY + y;
            
            if (fbX >= 0 && fbX < fbWidth && fbY >= 0 && fbY < fbHeight) {
                uint32_t pixel = tile.pixels[y * tile.width + x];
                // Skip transparent black pixels (0xFF000000)
                if (pixel != 0xFF000000) {
                    framebuffer[fbY * fbWidth + fbX] = pixel;
                }
            }
        }
    }
}

// Book's exact renderTilemap implementation
void renderTilemap(
    uint32_t* framebuffer, int fbWidth, int fbHeight,
    const TileMap& map, const Viewport& view
) {
    int startCol = view.xOffset / map.tileWidth;
    int startRow = view.yOffset / map.tileHeight;
    int xOffsetInTile = view.xOffset % map.tileWidth;
    int yOffsetInTile = view.yOffset % map.tileHeight;

    for (int y = 0; y <= view.height / map.tileHeight + 1; ++y) {
        for (int x = 0; x <= view.width / map.tileWidth + 1; ++x) {
            int mapCol = startCol + x;
            int mapRow = startRow + y;
            
            // Bounds checking
            if (mapCol < 0 || mapCol >= map.cols || mapRow < 0 || mapRow >= map.rows) {
                continue;
            }
            
            int mapIndex = mapRow * map.cols + mapCol;
            if (mapIndex >= (int)map.mapData.size()) continue;

            uint16_t tileIndex = map.mapData[mapIndex];
            if (tileIndex >= map.tileSet.size()) continue;
            
            const Tile& tile = map.tileSet[tileIndex];

            drawTileClipped(
                framebuffer, fbWidth, fbHeight,
                tile,
                x * map.tileWidth - xOffsetInTile,
                y * map.tileHeight - yOffsetInTile
            );
        }
    }
}

// Book's exact scrollViewport implementation
void scrollViewport(Viewport& view, int dx, int dy, int mapPixelWidth, int mapPixelHeight) {
    view.xOffset = std::clamp(view.xOffset + dx, 0, mapPixelWidth - view.width);
    view.yOffset = std::clamp(view.yOffset + dy, 0, mapPixelHeight - view.height);
}

// Create a test tilemap
TileMap createTestTileMap() {
    TileMap map;
    map.rows = 20;
    map.cols = 30;
    map.tileWidth = 16;
    map.tileHeight = 16;
    
    // Create tileset
    map.tileSet.push_back(createColorTile(16, 16, 0xFF228B22));    // Forest green
    map.tileSet.push_back(createColorTile(16, 16, 0xFF8B4513));    // Saddle brown  
    map.tileSet.push_back(createColorTile(16, 16, 0xFF4169E1));    // Royal blue
    map.tileSet.push_back(createColorTile(16, 16, 0xFFDC143C));    // Crimson
    map.tileSet.push_back(createPatternTile(16, 16, 0xFFFFD700, 0xFFFF8C00)); // Gold pattern
    map.tileSet.push_back(createPatternTile(16, 16, 0xFF9370DB, 0xFF4B0082)); // Purple pattern
    
    // Create map data with a pattern
    map.mapData.resize(map.rows * map.cols);
    
    std::mt19937 rng(42); // Fixed seed for reproducible maps
    std::uniform_int_distribution<int> tileDist(0, map.tileSet.size() - 1);
    
    for (int y = 0; y < map.rows; ++y) {
        for (int x = 0; x < map.cols; ++x) {
            int index = y * map.cols + x;
            
            // Create some structure - borders and random interior
            if (x == 0 || x == map.cols - 1 || y == 0 || y == map.rows - 1) {
                map.mapData[index] = 1; // Brown border
            } else if ((x + y) % 8 == 0) {
                map.mapData[index] = 4; // Gold pattern at regular intervals
            } else if ((x % 5 == 0) && (y % 5 == 0)) {
                map.mapData[index] = 5; // Purple pattern at grid points
            } else {
                map.mapData[index] = tileDist(rng) % 4; // Random from first 4 tiles
            }
        }
    }
    
    return map;
}

// Main game loop following book's example
int main(int argc, char** args) {
    bool quit = false;
    SDL_Surface* surface = NULL;
    SDL_Window* window = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return 1;
    }

    window = SDL_CreateWindow("Chapter 8: Tilemap System with Scrolling", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        return 1;
    }

    surface = SDL_GetWindowSurface(window);
    if (!surface) {
        cout << "Error getting surface: " << SDL_GetError() << endl;
        return 1;
    }

    // Create working surface in ARGB8888 format
    SDL_Surface* workingSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
    if (!workingSurface) {
        cout << "Error converting surface: " << SDL_GetError() << endl;
        return 1;
    }

    cout << "=== Chapter 8: Software Scrolling (Hardware-Like Tilemaps) ===" << endl;
    cout << "Surface: " << workingSurface->w << "x" << workingSurface->h << endl;
    cout << "Controls:" << endl;
    cout << "  Arrow Keys: Scroll viewport" << endl;
    cout << "  WASD: Faster scrolling" << endl;
    cout << "  ESC: Exit" << endl;

    // Create tilemap and viewport following book's example
    TileMap world = createTestTileMap();
    Viewport camera = {0, 0, workingSurface->w, workingSurface->h};
    
    int mapPixelWidth = world.cols * world.tileWidth;
    int mapPixelHeight = world.rows * world.tileHeight;
    
    cout << "Tilemap: " << world.cols << "x" << world.rows << " tiles" << endl;
    cout << "World size: " << mapPixelWidth << "x" << mapPixelHeight << " pixels" << endl;
    
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }

        // Handle input following book's example
        const bool* keys = SDL_GetKeyboardState(NULL);
        int scrollSpeed = 2;
        
        // Faster scrolling with WASD
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_A] || 
            keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_D]) {
            scrollSpeed = 4;
        }
        
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
            scrollViewport(camera, scrollSpeed, 0, mapPixelWidth, mapPixelHeight);
        }
        if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
            scrollViewport(camera, -scrollSpeed, 0, mapPixelWidth, mapPixelHeight);
        }
        if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
            scrollViewport(camera, 0, scrollSpeed, mapPixelWidth, mapPixelHeight);
        }
        if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
            scrollViewport(camera, 0, -scrollSpeed, mapPixelWidth, mapPixelHeight);
        }

        // Clear framebuffer following book's example
        SDL_LockSurface(workingSurface);
        uint32_t* framebuffer = (uint32_t*)workingSurface->pixels;
        int fbWidth = workingSurface->w;
        int fbHeight = workingSurface->h;
        
        memset(framebuffer, 0, fbWidth * fbHeight * sizeof(uint32_t));

        // Render tilemap to framebuffer following book's example
        renderTilemap(framebuffer, fbWidth, fbHeight, world, camera);
        
        SDL_UnlockSurface(workingSurface);

        // Present framebuffer to screen
        SDL_BlitSurface(workingSurface, NULL, surface, NULL);
        SDL_UpdateWindowSurface(window);
        
        // Simple FPS counter
        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            cout << "FPS: " << frameCount << " | Camera: (" << camera.xOffset 
                 << "," << camera.yOffset << ")" << endl;
            frameCount = 0;
            lastTime = currentTime;
        }

        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup tiles
    for (auto& tile : world.tileSet) {
        delete[] tile.pixels;
    }

    SDL_DestroySurface(workingSurface);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}