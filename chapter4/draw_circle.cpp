//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

void plotCirclePoints(int cx, int cy, int x, int y, uint32_t color, SDL_Surface* framebuffer, int stride){
  SDL_LockSurface(framebuffer);
  uint32_t* pixelPtr = (uint32_t*)framebuffer->pixels;

  auto setPixel = [&](int px, int py){
  if (px < 0 || py < 0){
    return;
  }
  int offset = py * stride + px;
  pixelPtr[offset] = color;
  };

  setPixel(cx + x, cy + y);
  setPixel(cx - x, cy + y);
  setPixel(cx + x, cy - y);
  setPixel(cx - x, cy - y);
  setPixel(cx + y, cy + x);
  setPixel(cx - y, cy + x);
  setPixel(cx + y, cy - x);
  setPixel(cx - y, cy - x);

  SDL_UnlockSurface(framebuffer);
}

void drawMidpointCircle(int cx, int cy, int radius, uint32_t color, SDL_Surface* framebuffer, int stride){
  int x = 0;
  int y = radius;
  int p = 1 - radius;
  plotCirclePoints(cx, cy, x, y, color, framebuffer, stride);

  while(x < y){
    x++;
    if(p < 0){
      p += 2 * x + 1;
    }
    else {
      y--;
      p += 2 * (x-y) + 1;
    }
    plotCirclePoints(cx, cy, x, y, color, framebuffer, stride);
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
  window = SDL_CreateWindow( "Draw Circle", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  //Draw a white circle at coordinates (640, 360) with radios 100
  drawMidpointCircle(640, 360, 100, 0xFFFFFFFF, surface, 1280);
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
