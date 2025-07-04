//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


void pixel_access(SDL_Surface* surface){
  uint32_t* pixels = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4;
  
  for(int y = 0; y < surface->h; ++y){
    for( int x = 0; x < surface->w; ++x){
      uint32_t pixel = pixels[y*pitch + x];
      uint8_t a = (pixel >> 24) & 0xFF;
      uint8_t r = (pixel >> 16) & 0xFF;
      uint8_t g = (pixel >> 8) & 0xFF;
      uint8_t b = pixel & 0xFF;
      // printing the pixels values crashes the program
      //cout << "a = " << a << " r = " << r << " g = " << g << " b = " << b << "\n";
    }
  }
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

  SDL_UpdateWindowSurface( window );
  
  // Direct memory access pixel reading
  pixel_access(surface);
  
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
