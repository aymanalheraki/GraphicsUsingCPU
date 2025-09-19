//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <cmath>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;


// Book's exact implementation of Bresenham's line algorithm
void drawLineBresenham(int x0, int y0, int x1, int y1, uint32_t color, 
                       uint8_t* framebuffer, int stride, int bytesPerPixel, int width, int height) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // Bounds checking and pixel setting
        if (x0 >= 0 && y0 >= 0 && x0 < width && y0 < height) {
            int offset = y0 * stride + x0 * bytesPerPixel;
            if (bytesPerPixel == 4) {
                uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(framebuffer + offset);
                *pixelPtr = color;
            }
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// SDL wrapper for the book's implementation
void drawLineBresenhamsSDL(int x0, int y0, int x1, int y1, uint32_t color, SDL_Surface* surface) {
    SDL_LockSurface(surface);
    uint8_t* framebuffer = (uint8_t*)surface->pixels;
    int stride = surface->pitch;
    int bytesPerPixel = SDL_BYTESPERPIXEL(surface->format);
    
    // Pass surface dimensions for bounds checking
    drawLineBresenham(x0, y0, x1, y1, color, framebuffer, stride, bytesPerPixel, surface->w, surface->h);
    SDL_UnlockSurface(surface);
}

void demo_bresenham_lines(SDL_Surface* surface) {
    // Clear to black
    SDL_LockSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    int pixelCount = (surface->pitch / 4) * surface->h;
    for(int i = 0; i < pixelCount; ++i) {
        pixels[i] = 0xFF000000; // Black with full alpha
    }
    SDL_UnlockSurface(surface);
    
    // Draw various lines to demonstrate Bresenham's algorithm
    int centerX = surface->w / 2;
    int centerY = surface->h / 2;
    
    cout << "Drawing from center (" << centerX << "," << centerY << ")" << endl;
    
    // Radial lines
    for(int angle = 0; angle < 360; angle += 30) {
        double rad = angle * M_PI / 180.0;
        int x1 = centerX + (int)(100 * cos(rad));
        int y1 = centerY + (int)(100 * sin(rad));
        cout << "Drawing radial line to (" << x1 << "," << y1 << ")" << endl;
        drawLineBresenhamsSDL(centerX, centerY, x1, y1, 0xFFFFFFFF, surface); // White
    }
    
    // Grid lines (vertical)
    for(int x = 50; x < surface->w; x += 50) {
        drawLineBresenhamsSDL(x, 0, x, surface->h - 1, 0xFF404040, surface); // Dark gray
    }
    // Grid lines (horizontal)
    for(int y = 50; y < surface->h; y += 50) {
        drawLineBresenhamsSDL(0, y, surface->w - 1, y, 0xFF404040, surface); // Dark gray
    }
    
    // Diagonal lines in different colors
    drawLineBresenhamsSDL(0, 0, surface->w - 1, surface->h - 1, 0xFFFF0000, surface); // Red
    drawLineBresenhamsSDL(surface->w - 1, 0, 0, surface->h - 1, 0xFF00FF00, surface); // Green
    
    cout << "Finished drawing all lines." << endl;
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

  // Get original surface info
  cout << "Original surface info:" << endl;
  cout << "  Format: " << SDL_GetPixelFormatName(surface->format) << endl;
  cout << "  Size: " << surface->w << "x" << surface->h << endl;
  cout << "  Pitch: " << surface->pitch << " bytes" << endl;
  cout << "  Bytes per pixel: " << SDL_BYTESPERPIXEL(surface->format) << endl;

  // Create a working surface in ARGB8888 format
  SDL_Surface* workingSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
  
  if (!workingSurface) {
    cout << "Error converting surface: " << SDL_GetError() << endl;
    return 1;
  }

  cout << "Working surface info:" << endl;
  cout << "  Format: " << SDL_GetPixelFormatName(workingSurface->format) << endl;

  cout << "Demonstrating Bresenham's Line Algorithm" << endl;
  cout << "Drawing radial lines, grid, and diagonals..." << endl;

  // Demonstrate Bresenham's line algorithm on working surface
  demo_bresenham_lines(workingSurface);

  // Copy the working surface to the window surface
  SDL_BlitSurface(workingSurface, NULL, surface, NULL);
  
  // Update the window to show our changes
  SDL_UpdateWindowSurface( window );
    
  while(!quit){
    SDL_WaitEvent(&event);
    if (event.type == SDL_EVENT_QUIT){
      quit = true;
    }
  }

  // Clean up the working surface
  SDL_DestroySurface(workingSurface);
  SDL_DestroyWindow( window );

  SDL_Quit();

  return 0;
}
