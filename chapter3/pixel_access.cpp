//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


void pixel_access_demo(SDL_Surface* surface){
  SDL_LockSurface(surface);
  uint32_t* pixels = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4; // pitch in pixels, not bytes
  
  cout << "Reading pixel data from framebuffer..." << endl;
  cout << "Format: ARGB8888" << endl;
  
  // Sample a few pixels and print their values
  for(int y = 0; y < min(10, surface->h); ++y){
    for(int x = 0; x < min(10, surface->w); ++x){
      uint32_t pixel = pixels[y * pitch + x];
      uint8_t a = (pixel >> 24) & 0xFF;
      uint8_t r = (pixel >> 16) & 0xFF;
      uint8_t g = (pixel >> 8) & 0xFF;
      uint8_t b = pixel & 0xFF;
      
      if (x < 5 && y < 5) { // Only print first few to avoid spam
        cout << "Pixel(" << x << "," << y << "): A=" << (int)a 
             << " R=" << (int)r << " G=" << (int)g << " B=" << (int)b << endl;
      }
      
      // Manipulate pixel colors here - brighten the image
      uint8_t new_r = min(255, (int)r + 50);
      uint8_t new_g = min(255, (int)g + 50);
      uint8_t new_b = min(255, (int)b + 50);
      
      uint32_t new_pixel = (a << 24) | (new_r << 16) | (new_g << 8) | new_b;
      pixels[y * pitch + x] = new_pixel;
    }
  }
  
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
  window = SDL_CreateWindow( "Pixel Access", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  // Convert framebuffer to ARGB8888 pixel format
  surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);

  // Fill surface with a test pattern first
  SDL_LockSurface(surface);
  uint32_t* pixels = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4;
  
  for(int y = 0; y < surface->h; ++y){
    for(int x = 0; x < surface->w; ++x){
      uint8_t r = (x * 255) / surface->w;
      uint8_t g = (y * 255) / surface->h;
      uint8_t b = 100;
      pixels[y * pitch + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
  }
  SDL_UnlockSurface(surface);

  SDL_UpdateWindowSurface( window );
  
  // Direct memory access pixel reading and manipulation
  pixel_access_demo(surface);
  
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
