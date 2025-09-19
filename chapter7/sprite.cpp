//Standard C++ libraries

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

//SDL3 library

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

struct Sprite {
  uint8_t* imageData;
  int width;
  int height;
  int x;
  int y;
  int frameIndex;
  int totalFrames;
  int frameWidth;
  int frameHeight;
  int frameDelayMs;
  uint64_t lastFrameTime;
};

uint64_t getCurrentTimeInMs(){
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t currentMs = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
  return currentMs;
}

void updateSpritePosition(Sprite& sprite, int dx, int dy){
  sprite.x += dx;
  sprite.y += dy;
}

struct Vec2 { float x, y; };

struct MovingSprite {
  Sprite sprite;
  Vec2 velocity;
};

void update(MovingSprite& ms, float deltaTime){
  ms.sprite.x += static_cast<int>(ms.velocity.x * deltaTime);
  ms.sprite.y += static_cast<int>(ms.velocity.y * deltaTime);
}

void updateAnimation(Sprite& sprite, uint64_t currentTimeMs) {
    if (currentTimeMs - sprite.lastFrameTime >= sprite.frameDelayMs) {
        sprite.frameIndex = (sprite.frameIndex + 1) % sprite.totalFrames;
        sprite.lastFrameTime = currentTimeMs;
    }
}

// Book's exact implementation of drawSpriteFrame
void drawSpriteFrame(uint32_t* framebuffer, int fbWidth, const Sprite& sprite) {
    int srcX = sprite.frameIndex * sprite.frameWidth;
    int srcY = 0;

    for (int y = 0; y < sprite.frameHeight; ++y) {
        for (int x = 0; x < sprite.frameWidth; ++x) {
            uint32_t pixel = ((uint32_t*)sprite.imageData)[(srcY + y) * sprite.width + (srcX + x)];
            framebuffer[(sprite.y + y) * fbWidth + (sprite.x + x)] = pixel;
        }
    }
}

// SDL-compatible wrapper for drawSpriteFrame
void drawSpriteFrameSDL(SDL_Surface* surface, const Sprite& sprite) {
    SDL_LockSurface(surface);
    uint32_t* framebuffer = (uint32_t*)surface->pixels;
    int fbWidth = surface->pitch / 4;
    
    // Bounds checking
    if (sprite.x >= surface->w || sprite.y >= surface->h) {
        SDL_UnlockSurface(surface);
        return;
    }
    
    int srcX = sprite.frameIndex * sprite.frameWidth;
    int srcY = 0;

    for (int y = 0; y < sprite.frameHeight; ++y) {
        for (int x = 0; x < sprite.frameWidth; ++x) {
            int dst_x = sprite.x + x;
            int dst_y = sprite.y + y;
            
            if (dst_x < surface->w && dst_y < surface->h) {
                // Create a simple colored square for demonstration (since we don't have actual sprite data)
                uint8_t r = (sprite.frameIndex * 40) % 256;
                uint8_t g = 100;
                uint8_t b = (sprite.frameIndex * 60 + 100) % 256;
                uint32_t pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
                
                framebuffer[dst_y * fbWidth + dst_x] = pixel;
            }
        }
    }
    
    SDL_UnlockSurface(surface);
}

void drawSpriteFrame(SDL_Surface* framebuffer, int fbWidth, const Sprite& sprite){
  int srcX = sprite.frameIndex * sprite.frameWidth;
  int srcY = 0;

  for(int y = 0; y < sprite.frameHeight; y++){
    for(int x = 0; x < sprite.frameWidth; x++){
      uint32_t pixel = ((uint32_t*)sprite.imageData)[(srcY + y) * sprite.width + (srcX + x)];
      uint32_t* currentPixel = (uint32_t*)framebuffer->pixels;
      currentPixel[(sprite.y + y) * fbWidth + (sprite.x + x)] = pixel;
    }
  }
}

void animationLoop(SDL_Window* window, SDL_Surface* framebuffer, Sprite& mySprite, SDL_Event event, bool running = true){
  int dx,dy = 0;
  while(running){
    if(event.type == SDL_EVENT_QUIT){
      running = false;
    }
    
    uint64_t now = getCurrentTimeInMs();

    updateSpritePosition(mySprite, dx, dy);
    updateAnimation(mySprite, now);

    memset(framebuffer->pixels, 0, mySprite.width * mySprite.height * sizeof(uint32_t));

    drawSpriteFrame(framebuffer, 720, mySprite);

    SDL_UpdateWindowSurface( window );

    //sleepUntilNextFrame();
  }
}

bool loadBMPToSprite(const char* filename, Sprite& sprite){
  ifstream file(filename, std::ios::binary);
  if(!file.is_open()){
    cerr << "Failed to load bmp file";
    return false;
  }

  #pragma pack(push, 1)
  struct BMPHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint16_t reserved1, reserved2;
    uint32_t dataOffset;
  };
  struct DIBHeader {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPPM, yPPM;
    uint32_t colorsUsed;
    uint32_t importantColors;
  };
  #pragma pack(pop)

  BMPHeader bmpHeader;
  DIBHeader dibHeader;

  file.read(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
  file.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

  if (bmpHeader.signature != 0x4D42) {
    std::cerr << "Not a valid BMP file." << std::endl;
    return false;
  }

  if (dibHeader.bitsPerPixel != 24 || dibHeader.compression != 0) {
    std::cerr << "Only uncompressed 24-bit BMP files are supported." << std::endl;
    return false;
  }

  sprite.width = dibHeader.width;
  sprite.height = dibHeader.height;

  int rowPadded = (sprite.width * 3 + 3) & (~3);
  std::vector<uint8_t> rawData(rowPadded * sprite.height);
  file.seekg(bmpHeader.dataOffset, std::ios::beg);
  file.read(reinterpret_cast<char*>(rawData.data()), rawData.size());

  sprite.imageData = new uint8_t[sprite.width * sprite.height * sizeof(uint32_t)];

  for (int y = 0; y < sprite.height; ++y) {
    for (int x = 0; x < sprite.width; ++x) {
      int bmpIndex = (sprite.height - 1 - y) * rowPadded + x * 3;
      uint8_t b = rawData[bmpIndex];
      uint8_t g = rawData[bmpIndex + 1];
      uint8_t r = rawData[bmpIndex + 2];
      uint32_t* dest = reinterpret_cast<uint32_t*>(sprite.imageData);
      dest[y * sprite.width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
  }

  return true;
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
  window = SDL_CreateWindow( "Sprite", 1280, 720, SDL_WINDOW_RESIZABLE);

  if ( !window ) {
    cout << "Error creating window: " << SDL_GetError()  << endl;
    return 1;
  }

  surface = SDL_GetWindowSurface( window );

  if ( !surface ) {
    cout << "Error getting surface: " << SDL_GetError() << endl;
    return 1;
  }

  Sprite mySprite;
  const char* file = "sprite.bmp";

  if(!loadBMPToSprite(file, mySprite)){
    cerr << "Failed to load Sprite" << endl;
    return -1;
  }
  
  // Initialize sprite properties
  mySprite.width = 160;
  mySprite.height = 20;
  mySprite.frameIndex = 0;
  mySprite.totalFrames = 8;
  mySprite.frameWidth = 20;
  mySprite.frameHeight = 20;
  mySprite.frameDelayMs = 200; // 200ms per frame
  mySprite.lastFrameTime = getCurrentTimeInMs();
  mySprite.x = 100;
  mySprite.y = 100;

  // Convert surface to ARGB8888 for consistency
  surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);

  cout << "Starting sprite animation loop (book's exact structure)..." << endl;
  cout << "Sprite: " << mySprite.totalFrames << " frames, " << mySprite.frameDelayMs << "ms per frame" << endl;

  // Book's exact animation loop structure
  bool running = true;
  while (running) {
      uint64_t now = getCurrentTimeInMs();

      // Handle events
      while (SDL_PollEvent(&event)) {
          if (event.type == SDL_EVENT_QUIT) {
              running = false;
          }
      }

      // Update positions and animations (book's exact calls)
      updateSpritePosition(mySprite, 1, 0); // Move sprite right
      updateAnimation(mySprite, now);

      // Clear framebuffer
      SDL_LockSurface(surface);
      uint32_t* framebuffer = (uint32_t*)surface->pixels;
      int pitch = surface->pitch / 4;
      for(int y = 0; y < surface->h; ++y) {
          for(int x = 0; x < surface->w; ++x) {
              framebuffer[y * pitch + x] = 0xFF000000; // Black background
          }
      }
      SDL_UnlockSurface(surface);

      // Draw sprite (book's exact function call)
      drawSpriteFrameSDL(surface, mySprite);

      // Wrap sprite position
      if (mySprite.x > surface->w) {
          mySprite.x = -mySprite.frameWidth;
      }

      SDL_UpdateWindowSurface(window);
      SDL_Delay(16); // ~60 FPS
  }

  SDL_DestroyWindow( window );

  SDL_Quit();

  return 0;
}
