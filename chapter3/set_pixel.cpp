//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


void setPixelARBG32(SDL_Surface* framebuffer, int stride, int x, int y, uint8_t a, uint8_t r, uint8_t g, uint8_t b){
  SDL_LockSurface(framebuffer);
  int bytesPerPixel = 4;
  int offset = y * stride + x * bytesPerPixel;
  uint32_t* pixelPtr = (uint32_t*)framebuffer->pixels;
  uint32_t pixelValue = (static_cast<uint32_t>(a) << 24)|
                        (static_cast<uint32_t>(r) << 16)|
                        (static_cast<uint32_t>(g) << 8)|
                        (static_cast<uint32_t>(b));
  pixelPtr[offset] = pixelValue;
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
  window = SDL_CreateWindow( "Set Pixel", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  // Directly set pixel with ARBG32
  setPixelARBG32(surface, 4, 25000, 20000, 255, 0, 255, 0);

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
