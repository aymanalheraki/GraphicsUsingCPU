//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

void BlitARGB32(
    uint32_t* dst, int dst_width, int dst_height, int dst_pitch,
    const uint32_t* src, int src_width, int src_height, int src_pitch,
    int dst_x, int dst_y)
{
    // Clipping
    int blit_width = src_width;
    int blit_height = src_height;
    if (dst_x < 0) { src += -dst_x; blit_width += dst_x; dst_x = 0; }
    if (dst_y < 0) { src += (-dst_y) * src_pitch; blit_height += dst_y; dst_y = 0; }
    if (dst_x + blit_width > dst_width) blit_width = dst_width - dst_x;
    if (dst_y + blit_height > dst_height) blit_height = dst_height - dst_y;

    for (int y = 0; y < blit_height; ++y) {
        uint32_t* dst_row = dst + (dst_y + y) * dst_pitch + dst_x;
        const uint32_t* src_row = src + y * src_pitch;
        for (int x = 0; x < blit_width; ++x) {
            dst_row[x] = src_row[x];  // Simple copy
        }
    }
}

void demo_blit(SDL_Surface* surface) {
    SDL_LockSurface(surface);
    
    // Create a small source pattern
    int src_width = 100;
    int src_height = 100;
    uint32_t* src_data = new uint32_t[src_width * src_height];
    
    // Fill source with a gradient pattern
    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            uint8_t r = (x * 255) / src_width;
            uint8_t g = (y * 255) / src_height;
            uint8_t b = 128;
            uint8_t a = 255;
            src_data[y * src_width + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    
    // Blit to surface
    uint32_t* dst = (uint32_t*)surface->pixels;
    int dst_pitch = surface->pitch / 4;
    
    BlitARGB32(dst, surface->w, surface->h, dst_pitch,
               src_data, src_width, src_height, src_width,
               50, 50);
    
    delete[] src_data;
    SDL_UnlockSurface(surface);
}

int main(int argc, char** args) {
  
  bool quit = false;
  SDL_Surface* surface = NULL;
  SDL_Window* window = NULL;
  SDL_Event event;

  if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    cout << "Error initializing SDL: " << SDL_GetError() << endl;
    return 1;
  } 
  window = SDL_CreateWindow( "Blit ARBG32", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  // Convert surface to ARGB8888 for consistency
  surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
  
  // Demonstrate blitting
  demo_blit(surface);
  SDL_UpdateWindowSurface( window );
    
  while(!quit){
    SDL_WaitEvent(&event);
    if (event.type == SDL_EVENT_QUIT){
      quit = true;
    }
  }

  SDL_DestroyWindow( window );

  SDL_Quit();

  return 0;
}
