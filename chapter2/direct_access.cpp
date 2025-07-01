//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


void direct_fill_blue(SDL_Surface* surface){
  SDL_LockSurface(surface);
  uint32_t* pixelsArray = (uint32_t*)surface->pixels;
  int pitch = surface->pitch / 4;
  
  for(int y = 0; y < surface->h; ++y){
    for( int x = 0; x < surface->w; ++x){
      pixelsArray[y * pitch + x] = 0xFF0000FF;
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

  // Direct memory access white filling
  direct_fill_blue(surface);

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
