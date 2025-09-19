//Standard C++ libraries

#include <unistd.h>
#include <iostream>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


// Book's exact implementation of setPixelARGB32
void setPixelARGB32(uint8_t* framebuffer, int stride, int x, int y, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    int bytesPerPixel = 4;
    int offset = y * stride + x * bytesPerPixel;
    uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(framebuffer + offset);
    
    uint32_t pixelValue = (static_cast<uint32_t>(a) << 24) |
                          (static_cast<uint32_t>(r) << 16) |
                          (static_cast<uint32_t>(g) << 8)  |
                          (static_cast<uint32_t>(b));
                          
    *pixelPtr = pixelValue;
}

// SDL-compatible wrapper
void setPixelSDL(SDL_Surface* surface, int x, int y, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
    
    SDL_LockSurface(surface);
    uint8_t* framebuffer = (uint8_t*)surface->pixels;
    int stride = surface->pitch;
    setPixelARGB32(framebuffer, stride, x, y, a, r, g, b);
    SDL_UnlockSurface(surface);
}

void demo_set_pixel(SDL_Surface* surface) {
    cout << "Drawing test pattern using setPixelARGB32..." << endl;
    
    // Clear to black first
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pitch = surface->pitch / 4;
    for(int y = 0; y < surface->h; ++y) {
        for(int x = 0; x < surface->w; ++x) {
            pixels[y * pitch + x] = 0xFF000000; // Black
        }
    }
    SDL_UnlockSurface(surface);
    
    // Draw colorful pattern
    for(int y = 50; y < surface->h - 50; y += 10) {
        for(int x = 50; x < surface->w - 50; x += 10) {
            uint8_t r = (x * 255) / surface->w;
            uint8_t g = (y * 255) / surface->h;
            uint8_t b = 128;
            setPixelSDL(surface, x, y, 255, r, g, b);
        }
    }
    
    // Draw a border
    for(int x = 0; x < surface->w; ++x) {
        setPixelSDL(surface, x, 0, 255, 255, 255, 255);      // Top border
        setPixelSDL(surface, x, surface->h-1, 255, 255, 255, 255); // Bottom border
    }
    for(int y = 0; y < surface->h; ++y) {
        setPixelSDL(surface, 0, y, 255, 255, 255, 255);      // Left border
        setPixelSDL(surface, surface->w-1, y, 255, 255, 255, 255); // Right border
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

  // Convert framebuffer to ARGB8888 pixel format for consistency
  surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);

  cout << "Surface format: " << SDL_GetPixelFormatName(surface->format) << endl;
  cout << "Stride: " << surface->pitch << " bytes" << endl;
  cout << "Dimensions: " << surface->w << "x" << surface->h << endl;

  // Demonstrate the book's setPixelARGB32 function
  demo_set_pixel(surface);

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
