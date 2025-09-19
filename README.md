# Graphics Programming Using the CPU Only

This repository contains C++ examples demonstrating graphics programming techniques using only the CPU (no GPU acceleration). Each example is an independent file that can be compiled and run on its own, implementing the **exact code examples from the book**.

## Book Structure

The examples are organized by chapters following the book "Graphics Programming Using the CPU Only (No GPU)". All examples have been reviewed and enhanced to match the book's exact implementations:

### Chapter 1: The History of Software Rendering
- **`chapter1/vga_mode13h_equivalent.cpp`** - Modern equivalent of VGA Mode 13h programming with authentic 256-color palette, plasma effects, and classic VGA demo features

### Chapter 2: Setting Up Your Environment  
- **`chapter2/direct_access.cpp`** - Enhanced direct pixel access demonstration with surface format information, gradient fills, and performance timing

### Chapter 3: Memory and Pixels: The Foundation
- **`chapter3/allocate_aligned_framebuffer.cpp`** - Memory-aligned framebuffer allocation using posix_memalign
- **`chapter3/endian_detect.cpp`** - Compile-time and runtime endianness detection
- **`chapter3/endian_swap.cpp`** - Byte swapping using std::byteswap for endianness conversion
- **`chapter3/pixel_access.cpp`** - Enhanced ARGB pixel reading, decomposition, and manipulation
- **`chapter3/set_pixel.cpp`** - Book's exact setPixelARGB32 implementation with stride handling

### Chapter 4: Drawing Primitives (Lines, Rectangles, Circles)
- **`chapter4/bresenhams.cpp`** - Book's exact Bresenham line algorithm implementation with framebuffer and stride parameters
- **`chapter4/draw_circle.cpp`** - Midpoint circle algorithm with 8-way symmetry
- **`chapter4/fill_circle.cpp`** - Efficient scanline circle filling algorithm

### Chapter 5: Image Operations
- **`chapter5/blitARGB32.cpp`** - Book's exact 32-bit ARGB blitting implementation with clipping
- **`chapter5/alpha_blending.cpp`** - **NEW**: Alpha blending implementation from the book with floating-point arithmetic

### Chapter 6: Text Rendering on the CPU
- **Note**: Chapter 6 focuses on theory and doesn't contain concrete code examples

### Chapter 7: Animation and Timing
- **`chapter7/sprite.cpp`** - Enhanced with book's exact sprite structure, drawSpriteFrame, and animation loop
- **`chapter7/sprite_animation.cpp`** - Complete sprite animation system with physics and timing
- **`chapter7/double_buffering.cpp`** - Double buffering implementation for flicker-free animation
- **`chapter7/precise_timing.cpp`** - High-precision frame timing and rate control

## Enhanced Features (Book-Accurate)

### ✅ **Exact Book Implementations**
- **VGA Mode 13h**: Authentic 256-color palette with classic plasma effects
- **Bresenham's Algorithm**: Exact function signature with framebuffer, stride, and bytesPerPixel
- **setPixelARGB32**: Book's exact implementation with proper stride calculation
- **Alpha Blending**: Floating-point blending formula from Chapter 5
- **Sprite Animation**: Book's exact sprite structure and drawSpriteFrame function
- **Animation Loop**: Book's exact loop structure with updateAnimation and updateSpritePosition

### 🔧 **Technical Accuracy**
- All examples use the **exact function signatures** from the book
- Proper **memory alignment** and **stride handling**
- **Endianness detection** using both compile-time and runtime methods
- **High-precision timing** using platform-specific APIs
- **Pixel format consistency** with ARGB8888
- **SDL3 compatibility** with proper surface format handling and blitting
- **Bounds checking** in all drawing algorithms to prevent memory violations

## Requirements

- **SDL3** development libraries
- **C++17** compatible compiler (g++, clang++, or MSVC)
- **pkg-config** for SDL3 linking (Linux/macOS)

### Installing SDL3 on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install libsdl3-dev pkg-config
```

## Compiled Binaries

All examples are pre-compiled and organized in the `bin/` directory:

```
bin/
├── chapter1/vga_mode13h_equivalent     # VGA Mode 13h demo
├── chapter2/direct_access              # Direct pixel access  
├── chapter3/                           # Memory & pixel operations (5 binaries)
├── chapter4/                           # Drawing primitives (3 binaries)
├── chapter5/                           # Image operations (2 binaries)
└── chapter7/                           # Animation & timing (4 binaries)
```

Run any example directly: `./bin/chapter4/bresenhams`

## Compilation

### Simple examples (no SDL3 dependency):
```bash
cd chapter3
g++ -o endian_detect endian_detect.cpp
g++ -o allocate_aligned_framebuffer allocate_aligned_framebuffer.cpp
```

### SDL3-based examples:
```bash
# Chapter 1 - VGA Mode 13h Demo
cd chapter1
g++ -o vga_mode13h_equivalent vga_mode13h_equivalent.cpp $(pkg-config --cflags --libs sdl3)

# Chapter 2 - Enhanced Direct Access
cd chapter2  
g++ -o direct_access direct_access.cpp $(pkg-config --cflags --libs sdl3)

# Chapter 4 - Bresenham's Algorithm  
cd chapter4
g++ -o bresenhams bresenhams.cpp $(pkg-config --cflags --libs sdl3)

# Chapter 5 - Alpha Blending
cd chapter5
g++ -o alpha_blending alpha_blending.cpp $(pkg-config --cflags --libs sdl3)

# Chapter 7 - Sprite Animation
cd chapter7
g++ -o sprite sprite.cpp $(pkg-config --cflags --libs sdl3)
g++ -o sprite_animation sprite_animation.cpp $(pkg-config --cflags --libs sdl3)
g++ -o double_buffering double_buffering.cpp $(pkg-config --cflags --libs sdl3)
```

## Running Examples

After compilation, run the executables:
```bash
./endian_detect                 # Shows endianness detection
./vga_mode13h_equivalent        # VGA Mode 13h with plasma effect
./bresenhams                    # Bresenham line algorithm demo
./alpha_blending               # Alpha blending demonstration
./sprite_animation             # Animated bouncing sprites
./double_buffering             # Smooth animation demo
```

## Book Implementation Highlights

### 🎯 **Pixel-Perfect Accuracy**
- **setPixelARGB32**: `setPixelARGB32(uint8_t* framebuffer, int stride, int x, int y, uint8_t a, uint8_t r, uint8_t g, uint8_t b)`
- **drawLineBresenham**: `drawLineBresenham(int x0, int y0, int x1, int y1, uint32_t color, uint8_t* framebuffer, int stride, int bytesPerPixel, int width, int height)`
- **Alpha Blending**: `R_out = (uint8_t)(R_s * alpha + R_d * (1.0f - alpha))`

### 📊 **Performance Considerations**
- Memory-aligned framebuffer allocation (32-byte alignment)
- Proper pitch/stride handling for row padding
- Efficient scanline algorithms for filled shapes
- High-resolution timing for consistent frame rates
- **Surface format compatibility** between different pixel formats (XRGB8888 ↔ ARGB8888)
- **Bounds checking** in drawing routines to prevent crashes and memory corruption

### 🎨 **Visual Effects**
- Classic VGA plasma effects with authentic color cycling
- Translucent alpha blending with proper color composition
- Smooth sprite animation with time-based movement
- Double buffering for flicker-free rendering

## Educational Value

These examples demonstrate fundamental concepts essential for:
- **Game Engine Development** - Core rendering pipeline understanding
- **Embedded Systems** - Resource-constrained graphics programming  
- **Retro Computing** - Classic graphics programming techniques
- **Computer Graphics Education** - Understanding GPU-free rendering
- **Performance Optimization** - CPU-based graphics optimization

## Advanced Topics (Chapters 8-14)

The book also covers advanced topics including:
- **Real-time 2D effects** and software scrolling (Chapter 8)
- **3D graphics mathematics** and rasterization (Chapter 9)
- **Software texture mapping** and lighting (Chapters 10-11)
- **Font rendering** and text layout (Chapter 6)
- **Audio integration** and multimedia (Chapters 12-13)
- **Performance optimization** techniques (Chapter 14)

## Platform Compatibility

- **✅ Linux**: All examples tested on Ubuntu with SDL3
- **✅ Windows**: Compatible with MinGW/MSVC and SDL3 development libraries  
- **✅ macOS**: Compatible with Xcode command line tools and SDL3 via Homebrew

---

*All code examples implement the exact algorithms and function signatures from the book "Graphics Programming Using the CPU Only (No GPU)" for maximum educational accuracy.*
Graphics Programming Using the CPU Book Codes
