//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <chrono>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

// Direct memory access - fill with solid color
void direct_fill_blue(SDL_Surface* surface){
  SDL_LockSurface(surface);
  uint32_t* pixelsArray = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4;
  
  for(int y = 0; y < surface->h; ++y){
    for( int x = 0; x < surface->w; ++x){
      pixelsArray[y * pitch + x] = 0xFF0000FF; // ARGB: Blue
    }
  }

  SDL_UnlockSurface(surface);
}

// Direct memory access - fill with gradient pattern
void direct_fill_gradient(SDL_Surface* surface){
  SDL_LockSurface(surface);
  uint32_t* pixelsArray = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4;
  
  for(int y = 0; y < surface->h; ++y){
    for( int x = 0; x < surface->w; ++x){
      uint8_t r = (x * 255) / surface->w;
      uint8_t g = (y * 255) / surface->h;
      uint8_t b = 128;
      uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
      pixelsArray[y * pitch + x] = color;
    }
  }

  SDL_UnlockSurface(surface);
}

// Demonstrate pixel format information
void print_surface_info(SDL_Surface* surface) {
  cout << "\n=== Surface Information ===" << endl;
  cout << "Width: " << surface->w << " pixels" << endl;
  cout << "Height: " << surface->h << " pixels" << endl;
  cout << "Pitch: " << surface->pitch << " bytes per row" << endl;
  cout << "Bytes per pixel: " << SDL_BYTESPERPIXEL(surface->format) << endl;
  cout << "Bits per pixel: " << SDL_BITSPERPIXEL(surface->format) << endl;
  cout << "Pixel format: " << SDL_GetPixelFormatName(surface->format) << endl;
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
  window = SDL_CreateWindow( "Direct Read and Write Pixels", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  // Print surface information (demonstrates environment understanding)
  print_surface_info(surface);

  // Convert to a known format for consistent pixel manipulation
  SDL_Surface* converted_surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
  if (!converted_surface) {
    cout << "Error converting surface: " << SDL_GetError() << endl;
    return 1;
  }

  cout << "\n=== After Format Conversion ===" << endl;
  print_surface_info(converted_surface);

  // Demonstrate different direct access methods
  cout << "\nFilling with blue..." << endl;
  direct_fill_blue(converted_surface);
  SDL_BlitSurface(converted_surface, NULL, surface, NULL);
  SDL_UpdateWindowSurface( window );
  
  SDL_Delay(2000); // Show blue for 2 seconds
  
  cout << "Filling with gradient..." << endl;
  direct_fill_gradient(converted_surface);
  SDL_BlitSurface(converted_surface, NULL, surface, NULL);
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
