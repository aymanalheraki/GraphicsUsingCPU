//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

void fillCircle(int cx, int cy, int radius, uint32_t color, SDL_Surface* framebuffer, int stride){
  for(int y = -radius; y <= radius; y++){
    int scanlineY = cy + y;
    if(scanlineY < 0) continue;
    if(scanlineY >= framebuffer->h) break;

    int xSpan = static_cast<int>(sqrt(radius * radius - y * y));
    int xStart = cx - xSpan;
    int xEnd = cx + xSpan;

    if(xStart < 0) xStart = 0;
    if(xEnd >= framebuffer->w) xEnd = framebuffer->w -1;

    for(int x = xStart; x <= xEnd; x++){
      int offset = scanlineY * stride + x;
      uint32_t* pixelPtr = (uint32_t*)framebuffer->pixels;
      pixelPtr[offset] = color;
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
  window = SDL_CreateWindow( "Fill Circle", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  //Fill a circle with white color
  fillCircle(640, 360, 100, 0xFFFFFFFF, surface, 1280);
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
