# Compilation Guide

This document provides detailed compilation instructions for all examples in the Graphics Programming Using the CPU Only repository.

## Compiled Binaries Location

All compiled examples are located in the `bin/` directory, organized by chapter:

```
bin/
├── chapter1/
│   └── vga_mode13h_equivalent          # VGA Mode 13h with plasma effects
├── chapter2/
│   └── direct_access                   # Direct pixel access demonstration
├── chapter3/
│   ├── allocate_aligned_framebuffer    # Memory alignment demo
│   ├── endian_detect                   # Endianness detection
│   ├── endian_swap                     # Byte swapping (requires C++23)
│   ├── pixel_access                    # ARGB pixel manipulation
│   └── set_pixel                       # setPixelARGB32 demonstration
├── chapter4/
│   ├── bresenhams                      # Bresenham's line algorithm
│   ├── draw_circle                     # Circle drawing algorithm
│   └── fill_circle                     # Filled circle algorithm
├── chapter5/
│   ├── blitARGB32                      # 32-bit ARGB blitting
│   └── alpha_blending                  # Alpha blending demonstration
└── chapter7/
    ├── sprite                          # Enhanced sprite system
    ├── sprite_animation                # Sprite animation with physics
    ├── double_buffering                # Double buffering demo
    └── precise_timing                  # High-precision timing
```

## Quick Compilation Commands

### All Examples (from repository root):
```bash
# Create bin directories and compile all examples
./compile_all.sh  # (if script exists)

# Or compile individually by chapter:
```

### Chapter 1 - VGA Mode 13h
```bash
cd chapter1
g++ -o ../bin/chapter1/vga_mode13h_equivalent vga_mode13h_equivalent.cpp $(pkg-config --cflags --libs sdl3)
```

### Chapter 2 - Direct Access
```bash
cd chapter2
g++ -o ../bin/chapter2/direct_access direct_access.cpp $(pkg-config --cflags --libs sdl3)
```

### Chapter 3 - Memory & Pixels
```bash
cd chapter3
# Standalone examples (no SDL3)
g++ -o ../bin/chapter3/allocate_aligned_framebuffer allocate_aligned_framebuffer.cpp
g++ -o ../bin/chapter3/endian_detect endian_detect.cpp
g++ -std=c++23 -o ../bin/chapter3/endian_swap endian_swap.cpp

# SDL3-based examples
g++ -o ../bin/chapter3/pixel_access pixel_access.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter3/set_pixel set_pixel.cpp $(pkg-config --cflags --libs sdl3)
```

### Chapter 4 - Drawing Primitives
```bash
cd chapter4
g++ -o ../bin/chapter4/bresenhams bresenhams.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter4/draw_circle draw_circle.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter4/fill_circle fill_circle.cpp $(pkg-config --cflags --libs sdl3)
```

### Chapter 5 - Image Operations
```bash
cd chapter5
g++ -o ../bin/chapter5/blitARGB32 blitARGB32.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter5/alpha_blending alpha_blending.cpp $(pkg-config --cflags --libs sdl3)
```

### Chapter 7 - Animation & Timing
```bash
cd chapter7
g++ -o ../bin/chapter7/sprite sprite.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter7/sprite_animation sprite_animation.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter7/double_buffering double_buffering.cpp $(pkg-config --cflags --libs sdl3)
g++ -o ../bin/chapter7/precise_timing precise_timing.cpp $(pkg-config --cflags --libs sdl3)
```

## Running Examples

### Console-based examples (no window):
```bash
./bin/chapter3/endian_detect              # Shows system endianness
./bin/chapter3/allocate_aligned_framebuffer # Memory allocation demo
./bin/chapter3/endian_swap                 # Byte swapping demonstration
```

### SDL3 window-based examples:
```bash
./bin/chapter1/vga_mode13h_equivalent      # VGA Mode 13h with plasma
./bin/chapter2/direct_access               # Direct pixel manipulation
./bin/chapter3/pixel_access                # ARGB pixel operations
./bin/chapter3/set_pixel                   # Pixel setting demonstration
./bin/chapter4/bresenhams                  # Line drawing algorithms
./bin/chapter4/draw_circle                 # Circle drawing
./bin/chapter4/fill_circle                 # Filled circles
./bin/chapter5/blitARGB32                  # Image blitting
./bin/chapter5/alpha_blending              # Alpha transparency
./bin/chapter7/sprite                      # Basic sprite handling
./bin/chapter7/sprite_animation            # Animated sprites
./bin/chapter7/double_buffering            # Smooth animation
./bin/chapter7/precise_timing              # Frame rate control
```

## Dependencies

### Required:
- **SDL3 development libraries**: `sudo apt install libsdl3-dev pkg-config`
- **C++17 compiler**: g++, clang++, or MSVC
- **C++23 support**: For `endian_swap.cpp` (uses `std::byteswap`)

### Platform-specific SDL3 installation:

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install libsdl3-dev pkg-config
```

#### Fedora/RHEL:
```bash
sudo dnf install SDL3-devel pkgconf-pkg-config
```

#### macOS (Homebrew):
```bash
brew install sdl3 pkg-config
```

#### Windows (MSYS2):
```bash
pacman -S mingw-w64-x86_64-SDL3 mingw-w64-x86_64-pkg-config
```

## Compilation Notes

### SDL3 API Fixes Applied:
- **Keyboard events**: Changed `event.key.keysym.sym` → `event.key.key`
- **Surface format**: Changed `surface->format->field` → `SDL_FUNCTION(surface->format)`
- **Pixel format access**: Updated to use SDL3 macros and functions
- **Surface conversion handling**: Proper blitting between converted surfaces and window surfaces
- **Bounds checking**: Added comprehensive bounds checking in drawing algorithms

### Critical Rendering Fixes:
- **Bresenham's Algorithm**: Fixed black screen issue by properly handling surface conversion and blitting
- **Surface Format Compatibility**: Handle XRGB8888 ↔ ARGB8888 format differences
- **Memory Safety**: Added bounds checking to prevent out-of-bounds pixel access
- **Window Surface Updates**: Ensure converted surfaces are properly blitted back to visible window surface

### Compiler Flags:
- **C++17**: Default standard for most examples
- **C++23**: Required for `endian_swap.cpp` (`std::byteswap`)
- **SDL3 linking**: Uses `pkg-config --cflags --libs sdl3`

### Binary Sizes:
- **Standalone examples**: ~12-14 KB
- **SDL3 examples**: ~14-27 KB (depending on complexity)
- **Total compiled size**: ~350 KB for all examples

## Testing Status

All examples have been successfully compiled and tested:

✅ **Chapter 1**: VGA Mode 13h equivalent with plasma effects  
✅ **Chapter 2**: Direct pixel access with surface information  
✅ **Chapter 3**: All memory/pixel examples (5 binaries)  
✅ **Chapter 4**: Drawing primitives (3 binaries)  
✅ **Chapter 5**: Image operations (2 binaries)  
✅ **Chapter 7**: Animation and timing (4 binaries)  

**Total**: 16 successfully compiled and tested examples

## Troubleshooting

### Common Issues:

1. **SDL3 not found**: Install SDL3 development packages
2. **C++23 errors**: Use `g++ -std=c++23` for endian_swap.cpp
3. **Linking errors**: Ensure pkg-config finds SDL3: `pkg-config --exists sdl3`
4. **Permission errors**: Make sure bin directories are writable

### Rendering Issues Fixed:

5. **Black screen in graphics examples**: 
   - **Problem**: SDL_ConvertSurface() creates disconnected surface
   - **Solution**: Use working surface + SDL_BlitSurface() to window surface
   - **Examples affected**: bresenhams.cpp, and other drawing examples

6. **Pink pixels or corruption**:
   - **Problem**: Out-of-bounds memory access in drawing algorithms
   - **Solution**: Added comprehensive bounds checking in pixel operations
   - **Function updated**: drawLineBresenham() now includes width/height bounds

7. **Surface format mismatches**:
   - **Problem**: XRGB8888 vs ARGB8888 format differences
   - **Solution**: Proper format conversion and compatibility handling
   - **Debug**: Surface info now printed to help diagnose format issues

### Verification:
```bash
# Check if SDL3 is properly installed
pkg-config --modversion sdl3

# Test a simple example
./bin/chapter3/endian_detect

# Test SDL3 window example (should show graphics, not black screen)
./bin/chapter4/bresenhams

# Test SDL3 window example (will open a window)
./bin/chapter1/vga_mode13h_equivalent
```

### Expected Visual Output:

**Console Examples:**
- `endian_detect`: Shows "little endian detected" messages  
- `allocate_aligned_framebuffer`: Silent success (no output)

**Graphics Window Examples:**
- `vga_mode13h_equivalent`: Animated plasma effect with classic VGA colors
- `bresenhams`: White radial lines, gray grid, red/green diagonals on black background
- `draw_circle` / `fill_circle`: Various circle drawing demonstrations  
- `alpha_blending`: Translucent color blending effects
- `sprite_animation`: Bouncing animated sprites with physics
- `double_buffering`: Smooth animation without flickering

**Debug Output (Normal):**
```
Surface info:
  Format: SDL_PIXELFORMAT_XRGB8888
  Size: 1280x720
  Pitch: 5120 bytes
  Bytes per pixel: 4
Working surface info:
  Format: SDL_PIXELFORMAT_ARGB8888
```

---

All examples implement the exact algorithms and function signatures from the book "Graphics Programming Using the CPU Only (No GPU)" for maximum educational accuracy.