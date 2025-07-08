//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


void drawLineBresenhams(int x0, int y0, int x1, int y1, uint32_t color, SDL_Surface* framebuffer, int stride){
  SDL_LockSurface(framebuffer);

  int dx = abs(x1-x0);
  int dy = abs(y1-y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  uint32_t* pixelPtr = (uint32_t*)framebuffer->pixels;

  while(true){
    int offset = y0 * stride + x0;
    pixelPtr[offset] = color;
    if(x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if(e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if(e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
  SDL_UnlockSurface(framebuffer);
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
  window = SDL_CreateWindow( "Draw Line Using Bresenhams Algorythm", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  // Draw line using brenham's line algorithm
  // Drawing white line from coordinates (100,100) to (500,500)
  drawLineBresenhams(100, 100, 500, 500, 0xFFFFFFFF, surface, 1280);
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
