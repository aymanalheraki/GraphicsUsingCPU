//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

// Alpha blending function from the book
void alphaBlendPixel(uint8_t R_s, uint8_t G_s, uint8_t B_s, uint8_t A_s,
                     uint8_t R_d, uint8_t G_d, uint8_t B_d, uint8_t A_d,
                     uint8_t& R_out, uint8_t& G_out, uint8_t& B_out, uint8_t& A_out) {
    float alpha = A_s / 255.0f;
    R_out = (uint8_t)(R_s * alpha + R_d * (1.0f - alpha));
    G_out = (uint8_t)(G_s * alpha + G_d * (1.0f - alpha));
    B_out = (uint8_t)(B_s * alpha + B_d * (1.0f - alpha));
    A_out = (uint8_t)(A_s + A_d * (1.0f - alpha)); // Optional: output alpha channel
}

// Alpha blend a rectangle onto a surface
void alphaBlendRect(SDL_Surface* surface, int x, int y, int width, int height, 
                    uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) {
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    
    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int px = x + dx;
            int py = y + dy;
            
            if (px >= 0 && px < surface->w && py >= 0 && py < surface->h) {
                uint32_t dst_pixel = pixels[py * pitch + px];
                
                // Extract destination components
                uint8_t dst_a = (dst_pixel >> 24) & 0xFF;
                uint8_t dst_r = (dst_pixel >> 16) & 0xFF;
                uint8_t dst_g = (dst_pixel >> 8) & 0xFF;
                uint8_t dst_b = dst_pixel & 0xFF;
                
                // Blend
                uint8_t out_r, out_g, out_b, out_a;
                alphaBlendPixel(r, g, b, alpha, dst_r, dst_g, dst_b, dst_a,
                               out_r, out_g, out_b, out_a);
                
                // Write back
                uint32_t new_pixel = (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
                pixels[py * pitch + px] = new_pixel;
            }
        }
    }
    
    SDL_UnlockSurface(surface);
}

void demo_alpha_blending(SDL_Surface* surface) {
    // Clear to white background
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    for(int y = 0; y < surface->h; ++y) {
        for(int x = 0; x < surface->w; ++x) {
            pixels[y * pitch + x] = 0xFFFFFFFF; // White
        }
    }
    SDL_UnlockSurface(surface);
    
    // Draw overlapping translucent rectangles
    alphaBlendRect(surface, 100, 100, 200, 150, 255, 0, 0, 128);    // Semi-transparent red
    alphaBlendRect(surface, 150, 125, 200, 150, 0, 255, 0, 128);    // Semi-transparent green
    alphaBlendRect(surface, 200, 150, 200, 150, 0, 0, 255, 128);    // Semi-transparent blue
    
    // Draw more rectangles with different alpha values
    alphaBlendRect(surface, 50, 50, 100, 100, 255, 255, 0, 64);     // Very transparent yellow
    alphaBlendRect(surface, 500, 50, 100, 100, 255, 0, 255, 192);   // Mostly opaque magenta
    alphaBlendRect(surface, 600, 150, 100, 100, 0, 255, 255, 255);  // Fully opaque cyan
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
    
    window = SDL_CreateWindow("Alpha Blending Demo", 800, 600, SDL_WINDOW_RESIZABLE);

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

    cout << "Demonstrating Alpha Blending" << endl;
    cout << "Drawing overlapping translucent rectangles..." << endl;

    // Demonstrate alpha blending
    demo_alpha_blending(surface);

    SDL_UpdateWindowSurface(window);
    
    while (!quit) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            quit = true;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}