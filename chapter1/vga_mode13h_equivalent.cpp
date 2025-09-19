//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

// VGA-style 6-bit RGB palette (64 colors per component)
struct VGAPalette {
    uint32_t colors[256];
    
    VGAPalette() {
        // Initialize with a classic VGA palette
        for (int i = 0; i < 256; ++i) {
            if (i < 16) {
                // Standard 16 EGA colors
                switch (i) {
                    case 0:  colors[i] = 0xFF000000; break; // Black
                    case 1:  colors[i] = 0xFF000080; break; // Dark Blue
                    case 2:  colors[i] = 0xFF008000; break; // Dark Green
                    case 3:  colors[i] = 0xFF008080; break; // Dark Cyan
                    case 4:  colors[i] = 0xFF800000; break; // Dark Red
                    case 5:  colors[i] = 0xFF800080; break; // Dark Magenta
                    case 6:  colors[i] = 0xFF808000; break; // Brown
                    case 7:  colors[i] = 0xFFC0C0C0; break; // Light Gray
                    case 8:  colors[i] = 0xFF808080; break; // Dark Gray
                    case 9:  colors[i] = 0xFF0000FF; break; // Bright Blue
                    case 10: colors[i] = 0xFF00FF00; break; // Bright Green
                    case 11: colors[i] = 0xFF00FFFF; break; // Bright Cyan
                    case 12: colors[i] = 0xFFFF0000; break; // Bright Red
                    case 13: colors[i] = 0xFFFF00FF; break; // Bright Magenta
                    case 14: colors[i] = 0xFFFFFF00; break; // Yellow
                    case 15: colors[i] = 0xFFFFFFFF; break; // White
                }
            } else {
                // Generate smooth gradient for remaining colors
                int r = ((i - 16) * 4) % 256;
                int g = ((i - 16) * 2) % 256;
                int b = ((i - 16) * 8) % 256;
                colors[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }
    }
};

static VGAPalette vga_palette;

// Modern equivalent of VGA Mode 13h put_pixel function
void put_pixel(SDL_Surface* surface, int x, int y, uint8_t color_index) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
    
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    pixels[y * pitch + x] = vga_palette.colors[color_index];
}

// Classic VGA demo: plasma effect
void draw_plasma_effect(SDL_Surface* surface, double time) {
    SDL_LockSurface(surface);
    
    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            // Plasma formula typical of VGA demos
            double value = sin(x * 0.04 + time) + 
                          sin(y * 0.03 + time * 1.5) + 
                          sin((x + y) * 0.02 + time * 0.5) +
                          sin(sqrt(x*x + y*y) * 0.02 + time * 2.0);
            
            uint8_t color = (uint8_t)((value + 4.0) * 32.0) % 256;
            put_pixel(surface, x, y, color);
        }
    }
    
    SDL_UnlockSurface(surface);
}

// Draw classic VGA-style text (8x8 bitmap font simulation)
void draw_text(SDL_Surface* surface, const char* text, int start_x, int start_y, uint8_t color) {
    SDL_LockSurface(surface);
    
    int x = start_x;
    int y = start_y;
    
    for (const char* c = text; *c; ++c) {
        if (*c == '\n') {
            x = start_x;
            y += 9;
            continue;
        }
        
        // Simple 8x8 character rendering (just bars for demo)
        for (int dy = 0; dy < 8; ++dy) {
            for (int dx = 0; dx < 8; ++dx) {
                // Simple pattern based on character
                if ((*c * (dx + 1) + dy) % 3 == 0) {
                    put_pixel(surface, x + dx, y + dy, color);
                }
            }
        }
        x += 9; // Move to next character position
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
    
    // Create window with authentic VGA Mode 13h resolution (320x200) scaled 2x
    window = SDL_CreateWindow("VGA Mode 13h Demo - 320x200", 640, 400, SDL_WINDOW_RESIZABLE);

    if (!window) {
        cout << "Error creating window: " << SDL_GetError() << endl;
        return 1;
    }

    surface = SDL_GetWindowSurface(window);

    if (!surface) {
        cout << "Error getting surface: " << SDL_GetError() << endl;
        return 1;
    }

    // Convert to ARGB8888 format for consistent pixel manipulation
    surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);

    cout << "VGA Mode 13h Equivalent Demo" << endl;
    cout << "Resolution: " << surface->w << "x" << surface->h << endl;
    cout << "Emulating 256-color palette" << endl;
    cout << "Press ESC or close window to exit" << endl;

    uint64_t start_time = SDL_GetTicks();
    
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
        
        double time = (SDL_GetTicks() - start_time) / 1000.0;
        
        // Draw animated plasma effect (classic VGA demo style)
        draw_plasma_effect(surface, time);
        
        // Draw title text
        draw_text(surface, "VGA MODE 13h DEMO", 10, 10, 15); // White text
        draw_text(surface, "320x200 256 COLORS", 10, 25, 14); // Yellow text
        
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}