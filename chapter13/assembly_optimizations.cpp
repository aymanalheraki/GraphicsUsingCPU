//Chapter 13: Using Assembly for Performance 
//Standard C++ libraries
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <immintrin.h>  // For SIMD intrinsics comparison

using namespace std;
using namespace std::chrono;

// Function prototypes for different optimization levels
extern "C" {
    void blit_simple_c(uint32_t* dst, const uint32_t* src, size_t count);
    void blit_unrolled_c(uint32_t* dst, const uint32_t* src, size_t count);
    void blit_inline_asm(uint32_t* dst, const uint32_t* src, size_t count);
    void memfill_asm(uint8_t* dst, size_t count, uint8_t value);
    void alpha_blend_asm(uint32_t* dst, const uint32_t* src, size_t count, uint8_t alpha);
}

// Book's simple C implementation for comparison
void blit_simple_c(uint32_t* dst, const uint32_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

// Book's manual loop unrolling in C
void blit_unrolled_c(uint32_t* dst, const uint32_t* src, size_t count) {
    size_t unrolled = count & ~7; // Process in groups of 8
    size_t i = 0;
    
    // Unrolled loop - process 8 pixels at once
    for (; i < unrolled; i += 8) {
        dst[i] = src[i];
        dst[i+1] = src[i+1];
        dst[i+2] = src[i+2];
        dst[i+3] = src[i+3];
        dst[i+4] = src[i+4];
        dst[i+5] = src[i+5];
        dst[i+6] = src[i+6];
        dst[i+7] = src[i+7];
    }
    
    // Handle remaining pixels
    for (; i < count; ++i) {
        dst[i] = src[i];
    }
}

// Book's inline assembly implementation
void blit_inline_asm(uint32_t* dst, const uint32_t* src, size_t count) {
    #ifdef __x86_64__
    // Use inline assembly with Intel syntax
    asm volatile (
        ".intel_syntax noprefix\n\t"
        "mov rax, %0\n\t"           // Load count
        "mov rsi, %1\n\t"           // Load src
        "mov rdi, %2\n\t"           // Load dst
        "shr rax, 3\n\t"            // Divide by 8 for unrolling
        "test rax, rax\n\t"
        "jz 2f\n\t"                 // Jump if no full blocks
        
        "1:\n\t"                    // Main loop - copy 8 pixels at once
        "mov r8, [rsi]\n\t"         // Load 2 pixels (8 bytes)
        "mov r9, [rsi+8]\n\t"
        "mov r10, [rsi+16]\n\t"
        "mov r11, [rsi+24]\n\t"
        "mov [rdi], r8\n\t"         // Store 2 pixels
        "mov [rdi+8], r9\n\t"
        "mov [rdi+16], r10\n\t"
        "mov [rdi+24], r11\n\t"
        "add rsi, 32\n\t"           // Advance source by 32 bytes (8 pixels)
        "add rdi, 32\n\t"           // Advance destination by 32 bytes
        "dec rax\n\t"
        "jnz 1b\n\t"
        
        "2:\n\t"                    // Handle remaining pixels
        "mov rax, %0\n\t"
        "and rax, 7\n\t"            // Get remainder
        "test rax, rax\n\t"
        "jz 4f\n\t"
        
        "3:\n\t"
        "mov r8d, [rsi]\n\t"        // Copy remaining pixels one by one
        "mov [rdi], r8d\n\t"
        "add rsi, 4\n\t"
        "add rdi, 4\n\t"
        "dec rax\n\t"
        "jnz 3b\n\t"
        
        "4:\n\t"
        ".att_syntax prefix\n\t"
        :
        : "r"(count), "r"(src), "r"(dst)
        : "rax", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
    #else
    // Fallback to C implementation for non-x86_64
    blit_unrolled_c(dst, src, count);
    #endif
}

// Book's memory fill assembly function
void memfill_asm(uint8_t* dst, size_t count, uint8_t value) {
    #ifdef __x86_64__
    asm volatile (
        ".intel_syntax noprefix\n\t"
        "mov rdi, %0\n\t"           // Destination
        "mov rcx, %1\n\t"           // Count
        "mov al, %2\n\t"            // Value to fill
        "rep stosb\n\t"             // Fill memory
        ".att_syntax prefix\n\t"
        :
        : "r"(dst), "r"(count), "r"(value)
        : "rdi", "rcx", "rax", "memory"
    );
    #else
    memset(dst, value, count);
    #endif
}

// Book's alpha blending with assembly optimization
void alpha_blend_asm(uint32_t* dst, const uint32_t* src, size_t count, uint8_t alpha) {
    #ifdef __x86_64__
    // Use SSE2 for alpha blending
    __m128i alpha_vec = _mm_set1_epi16(alpha);
    __m128i inv_alpha_vec = _mm_set1_epi16(255 - alpha);
    
    size_t simd_count = count & ~3; // Process 4 pixels at once
    
    for (size_t i = 0; i < simd_count; i += 4) {
        // Load source and destination pixels
        __m128i src_pixels = _mm_loadu_si128((__m128i*)(src + i));
        __m128i dst_pixels = _mm_loadu_si128((__m128i*)(dst + i));
        
        // Unpack to 16-bit for multiplication
        __m128i src_lo = _mm_unpacklo_epi8(src_pixels, _mm_setzero_si128());
        __m128i src_hi = _mm_unpackhi_epi8(src_pixels, _mm_setzero_si128());
        __m128i dst_lo = _mm_unpacklo_epi8(dst_pixels, _mm_setzero_si128());
        __m128i dst_hi = _mm_unpackhi_epi8(dst_pixels, _mm_setzero_si128());
        
        // Perform alpha blending: dst = src * alpha + dst * (255 - alpha)
        __m128i blended_lo = _mm_add_epi16(
            _mm_mullo_epi16(src_lo, alpha_vec),
            _mm_mullo_epi16(dst_lo, inv_alpha_vec)
        );
        __m128i blended_hi = _mm_add_epi16(
            _mm_mullo_epi16(src_hi, alpha_vec),
            _mm_mullo_epi16(dst_hi, inv_alpha_vec)
        );
        
        // Divide by 255 (approximate with >> 8)
        blended_lo = _mm_srli_epi16(blended_lo, 8);
        blended_hi = _mm_srli_epi16(blended_hi, 8);
        
        // Pack back to 8-bit
        __m128i result = _mm_packus_epi16(blended_lo, blended_hi);
        
        // Store result
        _mm_storeu_si128((__m128i*)(dst + i), result);
    }
    
    // Handle remaining pixels
    for (size_t i = simd_count; i < count; ++i) {
        uint32_t src_pixel = src[i];
        uint32_t dst_pixel = dst[i];
        
        uint8_t src_r = (src_pixel >> 16) & 0xFF;
        uint8_t src_g = (src_pixel >> 8) & 0xFF;
        uint8_t src_b = src_pixel & 0xFF;
        
        uint8_t dst_r = (dst_pixel >> 16) & 0xFF;
        uint8_t dst_g = (dst_pixel >> 8) & 0xFF;
        uint8_t dst_b = dst_pixel & 0xFF;
        
        uint8_t result_r = (src_r * alpha + dst_r * (255 - alpha)) / 255;
        uint8_t result_g = (src_g * alpha + dst_g * (255 - alpha)) / 255;
        uint8_t result_b = (src_b * alpha + dst_b * (255 - alpha)) / 255;
        
        dst[i] = (0xFF << 24) | (result_r << 16) | (result_g << 8) | result_b;
    }
    #else
    // Fallback C implementation
    for (size_t i = 0; i < count; ++i) {
        uint32_t src_pixel = src[i];
        uint32_t dst_pixel = dst[i];
        
        uint8_t src_r = (src_pixel >> 16) & 0xFF;
        uint8_t src_g = (src_pixel >> 8) & 0xFF;
        uint8_t src_b = src_pixel & 0xFF;
        
        uint8_t dst_r = (dst_pixel >> 16) & 0xFF;
        uint8_t dst_g = (dst_pixel >> 8) & 0xFF;
        uint8_t dst_b = dst_pixel & 0xFF;
        
        uint8_t result_r = (src_r * alpha + dst_r * (255 - alpha)) / 255;
        uint8_t result_g = (src_g * alpha + dst_g * (255 - alpha)) / 255;
        uint8_t result_b = (src_b * alpha + dst_b * (255 - alpha)) / 255;
        
        dst[i] = (0xFF << 24) | (result_r << 16) | (result_g << 8) | result_b;
    }
    #endif
}

// Performance testing framework
class AssemblyPerformanceTester {
private:
    vector<uint32_t> src_data;
    vector<uint32_t> dst_data;
    vector<uint8_t> mem_data;
    
public:
    AssemblyPerformanceTester(size_t size) {
        // Initialize test data
        src_data.resize(size);
        dst_data.resize(size);
        mem_data.resize(size * 4);
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
        
        for (size_t i = 0; i < size; ++i) {
            src_data[i] = dis(gen);
            dst_data[i] = dis(gen);
        }
    }
    
    void testBlitPerformance() {
        cout << "\n=== Pixel Blitting Performance Test ===" << endl;
        
        const int iterations = 1000;
        const size_t pixel_count = src_data.size();
        
        // Test simple C version
        auto dst_copy = dst_data;
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            blit_simple_c(dst_copy.data(), src_data.data(), pixel_count);
        }
        auto simple_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        // Test unrolled C version
        dst_copy = dst_data;
        start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            blit_unrolled_c(dst_copy.data(), src_data.data(), pixel_count);
        }
        auto unrolled_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        // Test inline assembly version
        dst_copy = dst_data;
        start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            blit_inline_asm(dst_copy.data(), src_data.data(), pixel_count);
        }
        auto asm_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        // Test memcpy for comparison
        dst_copy = dst_data;
        start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            memcpy(dst_copy.data(), src_data.data(), pixel_count * sizeof(uint32_t));
        }
        auto memcpy_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        cout << "Blitting " << pixel_count << " pixels × " << iterations << " iterations:" << endl;
        cout << "Simple C:     " << simple_time << " μs (1.00x)" << endl;
        cout << "Unrolled C:   " << unrolled_time << " μs (" << fixed << setprecision(2) << (double)simple_time/unrolled_time << "x)" << endl;
        cout << "Inline ASM:   " << asm_time << " μs (" << (double)simple_time/asm_time << "x)" << endl;
        cout << "memcpy:       " << memcpy_time << " μs (" << (double)simple_time/memcpy_time << "x)" << endl;
    }
    
    void testMemfillPerformance() {
        cout << "\n=== Memory Fill Performance Test ===" << endl;
        
        const int iterations = 1000;
        const size_t byte_count = mem_data.size();
        
        // Test standard memset
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            memset(mem_data.data(), 0x42, byte_count);
        }
        auto memset_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        // Test assembly memfill
        start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            memfill_asm(mem_data.data(), byte_count, 0x42);
        }
        auto asm_fill_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        cout << "Filling " << byte_count << " bytes × " << iterations << " iterations:" << endl;
        cout << "memset:       " << memset_time << " μs (1.00x)" << endl;
        cout << "Assembly:     " << asm_fill_time << " μs (" << fixed << setprecision(2) << (double)memset_time/asm_fill_time << "x)" << endl;
    }
    
    void testAlphaBlendPerformance() {
        cout << "\n=== Alpha Blending Performance Test ===" << endl;
        
        const int iterations = 100;
        const size_t pixel_count = src_data.size();
        const uint8_t alpha = 128; // 50% transparency
        
        // Test assembly alpha blend
        auto dst_copy = dst_data;
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            alpha_blend_asm(dst_copy.data(), src_data.data(), pixel_count, alpha);
        }
        auto asm_blend_time = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        
        cout << "Alpha blending " << pixel_count << " pixels × " << iterations << " iterations:" << endl;
        cout << "Assembly/SSE: " << asm_blend_time << " μs" << endl;
        cout << "Throughput:   " << fixed << setprecision(1) << (pixel_count * iterations * 1000000.0 / asm_blend_time / 1000000.0) << " MP/s" << endl;
    }
};

void demonstrateIntelVsATTSyntax() {
    cout << "\n=== Intel vs AT&T Syntax Demonstration ===" << endl;
    
    cout << "Intel Syntax Examples:" << endl;
    cout << "  mov eax, ebx        ; Move ebx to eax" << endl;
    cout << "  mov eax, 5          ; Move immediate 5 to eax" << endl;
    cout << "  mov eax, [ebx+4]    ; Move memory at ebx+4 to eax" << endl;
    cout << "  add eax, 1          ; Add 1 to eax" << endl;
    
    cout << "\nAT&T Syntax Examples:" << endl;
    cout << "  movl %ebx, %eax     ; Same as above" << endl;
    cout << "  movl $5, %eax       ; Same as above" << endl;
    cout << "  movl 4(%ebx), %eax  ; Same as above" << endl;
    cout << "  addl $1, %eax       ; Same as above" << endl;
    
    cout << "\nKey Differences:" << endl;
    cout << "• Intel: destination first, source second" << endl;
    cout << "• AT&T: source first, destination second" << endl;
    cout << "• Intel: bare registers, square brackets for memory" << endl;
    cout << "• AT&T: % prefix for registers, parentheses for memory" << endl;
    cout << "• AT&T: $ prefix for immediates, instruction suffixes for size" << endl;
}

void demonstrateLoopUnrolling() {
    cout << "\n=== Loop Unrolling Concepts ===" << endl;
    
    cout << "Basic Loop (processes 1 pixel per iteration):" << endl;
    cout << "  for (i = 0; i < count; ++i)" << endl;
    cout << "    dst[i] = src[i];" << endl;
    cout << "  Overhead: branch + counter increment per pixel" << endl;
    
    cout << "\nUnrolled Loop (processes 8 pixels per iteration):" << endl;
    cout << "  for (i = 0; i < count; i += 8) {" << endl;
    cout << "    dst[i] = src[i];     dst[i+1] = src[i+1];" << endl;
    cout << "    dst[i+2] = src[i+2]; dst[i+3] = src[i+3];" << endl;
    cout << "    dst[i+4] = src[i+4]; dst[i+5] = src[i+5];" << endl;
    cout << "    dst[i+6] = src[i+6]; dst[i+7] = src[i+7];" << endl;
    cout << "  }" << endl;
    cout << "  Benefits: 8x fewer branches, better instruction pipelining" << endl;
    
    cout << "\nAssembly Benefits:" << endl;
    cout << "• Manual register allocation" << endl;
    cout << "• Optimal instruction scheduling" << endl;
    cout << "• SIMD instruction usage" << endl;
    cout << "• Elimination of array bounds checking" << endl;
}

int main(int argc, char** args) {
    cout << "=== Chapter 13: Using Assembly for Performance ===" << endl;
    cout << "Demonstrating assembly optimization techniques for CPU graphics" << endl;
    
    // Architecture detection
    cout << "\nTarget Architecture: ";
    #ifdef __x86_64__
        cout << "x86_64 (64-bit)" << endl;
    #elif defined(__i386__)
        cout << "x86 (32-bit)" << endl;
    #elif defined(__ARM_ARCH)
        cout << "ARM" << endl;
    #else
        cout << "Unknown/Generic" << endl;
    #endif
    
    // Demonstrate syntax differences
    demonstrateIntelVsATTSyntax();
    
    // Demonstrate loop unrolling concepts
    demonstrateLoopUnrolling();
    
    // Performance testing
    const size_t test_size = 1024 * 1024; // 1M pixels
    AssemblyPerformanceTester tester(test_size);
    
    tester.testBlitPerformance();
    tester.testMemfillPerformance(); 
    tester.testAlphaBlendPerformance();
    
    cout << "\n=== Assembly Optimization Summary ===" << endl;
    cout << "✓ Loop unrolling reduces branch overhead" << endl;
    cout << "✓ Inline assembly provides fine-grained control" << endl;
    cout << "✓ SIMD instructions accelerate parallel operations" << endl;
    cout << "✓ Manual optimization beats compiler in specific cases" << endl;
    cout << "✓ Assembly crucial for maximum CPU graphics performance" << endl;
    
    cout << "\nWhen to use assembly:" << endl;
    cout << "• Performance-critical inner loops" << endl;
    cout << "• SIMD operations not expressible in C++" << endl;
    cout << "• Platform-specific optimizations" << endl;
    cout << "• Memory layout control requirements" << endl;
    
    return 0;
}