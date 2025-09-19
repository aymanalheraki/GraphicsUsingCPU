//Chapter 10: Optimizations - SIMD Instructions (SSE, AVX)
//Standard C++ libraries
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <random>
#include <cstring>
#include <cstdint>
#include <algorithm>

//SIMD intrinsics
#include <emmintrin.h>  // SSE2
#include <immintrin.h>  // AVX/AVX2

using namespace std;
using namespace std::chrono;

//Book's exact alpha blending implementations
//Scalar (non-SIMD) version
void alpha_blend_scalar(uint8_t* dst, const uint8_t* src, uint8_t alpha, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = (src[i] * alpha + dst[i] * (255 - alpha)) / 255;
    }
}

//SSE2 optimized version - processes 16 pixels at once
void alpha_blend_sse(uint8_t* dst, const uint8_t* src, uint8_t alpha, size_t count) {
    __m128i src_pixels, dst_pixels;
    __m128i alpha_vec = _mm_set1_epi16(alpha);
    __m128i inv_alpha_vec = _mm_set1_epi16(255 - alpha);

    size_t i = 0;
    for (; i + 15 < count; i += 16) {
        // Load 16 bytes (pixels) from src and dst
        src_pixels = _mm_loadu_si128((__m128i*)(src + i));
        dst_pixels = _mm_loadu_si128((__m128i*)(dst + i));

        // Unpack bytes to words to prevent overflow during multiplication
        __m128i src_lo = _mm_unpacklo_epi8(src_pixels, _mm_setzero_si128());
        __m128i src_hi = _mm_unpackhi_epi8(src_pixels, _mm_setzero_si128());
        __m128i dst_lo = _mm_unpacklo_epi8(dst_pixels, _mm_setzero_si128());
        __m128i dst_hi = _mm_unpackhi_epi8(dst_pixels, _mm_setzero_si128());

        // Multiply and blend low bytes
        __m128i blended_lo = _mm_add_epi16(
            _mm_mullo_epi16(src_lo, alpha_vec),
            _mm_mullo_epi16(dst_lo, inv_alpha_vec)
        );

        // Multiply and blend high bytes
        __m128i blended_hi = _mm_add_epi16(
            _mm_mullo_epi16(src_hi, alpha_vec),
            _mm_mullo_epi16(dst_hi, inv_alpha_vec)
        );

        // Divide by 255 approximation using (value + 128 + (value >> 8)) >> 8 for efficiency
        blended_lo = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(blended_lo, _mm_set1_epi16(128)), _mm_srli_epi16(blended_lo, 8)), 8);
        blended_hi = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(blended_hi, _mm_set1_epi16(128)), _mm_srli_epi16(blended_hi, 8)), 8);

        // Pack words back to bytes
        __m128i blended_pixels = _mm_packus_epi16(blended_lo, blended_hi);

        // Store result
        _mm_storeu_si128((__m128i*)(dst + i), blended_pixels);
    }

    // Process remaining pixels scalar way
    for (; i < count; ++i) {
        dst[i] = (src[i] * alpha + dst[i] * (255 - alpha)) / 255;
    }
}

//AVX2 optimized version - processes 32 pixels at once
void alpha_blend_avx2(uint8_t* dst, const uint8_t* src, uint8_t alpha, size_t count) {
    __m256i alpha_vec = _mm256_set1_epi16(alpha);
    __m256i inv_alpha_vec = _mm256_set1_epi16(255 - alpha);

    size_t i = 0;
    for (; i + 31 < count; i += 32) {
        // Load 32 bytes (pixels) from src and dst
        __m256i src_pixels = _mm256_loadu_si256((__m256i*)(src + i));
        __m256i dst_pixels = _mm256_loadu_si256((__m256i*)(dst + i));

        // Unpack bytes to words to prevent overflow during multiplication
        __m256i src_lo = _mm256_unpacklo_epi8(src_pixels, _mm256_setzero_si256());
        __m256i src_hi = _mm256_unpackhi_epi8(src_pixels, _mm256_setzero_si256());
        __m256i dst_lo = _mm256_unpacklo_epi8(dst_pixels, _mm256_setzero_si256());
        __m256i dst_hi = _mm256_unpackhi_epi8(dst_pixels, _mm256_setzero_si256());

        // Multiply and blend
        __m256i blended_lo = _mm256_add_epi16(
            _mm256_mullo_epi16(src_lo, alpha_vec),
            _mm256_mullo_epi16(dst_lo, inv_alpha_vec)
        );

        __m256i blended_hi = _mm256_add_epi16(
            _mm256_mullo_epi16(src_hi, alpha_vec),
            _mm256_mullo_epi16(dst_hi, inv_alpha_vec)
        );

        // Divide by 255 approximation
        blended_lo = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(blended_lo, _mm256_set1_epi16(128)), _mm256_srli_epi16(blended_lo, 8)), 8);
        blended_hi = _mm256_srli_epi16(_mm256_add_epi16(_mm256_add_epi16(blended_hi, _mm256_set1_epi16(128)), _mm256_srli_epi16(blended_hi, 8)), 8);

        // Pack words back to bytes
        __m256i blended_pixels = _mm256_packus_epi16(blended_lo, blended_hi);

        // Store result
        _mm256_storeu_si256((__m256i*)(dst + i), blended_pixels);
    }

    // Process remaining pixels with SSE or scalar
    alpha_blend_sse(dst + i, src + i, alpha, count - i);
}

//Color manipulation functions
void grayscale_scalar(uint8_t* rgb_data, size_t pixel_count) {
    for (size_t i = 0; i < pixel_count; i += 3) {
        uint8_t gray = (rgb_data[i] * 77 + rgb_data[i+1] * 150 + rgb_data[i+2] * 29) >> 8;
        rgb_data[i] = rgb_data[i+1] = rgb_data[i+2] = gray;
    }
}

void grayscale_sse(uint8_t* rgb_data, size_t pixel_count) {
    __m128i coeff_r = _mm_set1_epi16(77);
    __m128i coeff_g = _mm_set1_epi16(150);
    __m128i coeff_b = _mm_set1_epi16(29);

    size_t i = 0;
    // Process 5 RGB pixels at a time (15 bytes, but work with 16 bytes)
    for (; i + 14 < pixel_count; i += 15) {
        // This is a simplified version - real implementation would handle RGB interleaving
        __m128i pixels = _mm_loadu_si128((__m128i*)(rgb_data + i));
        
        // For demonstration, we'll do a simple average (not exact grayscale formula)
        __m128i unpacked_lo = _mm_unpacklo_epi8(pixels, _mm_setzero_si128());
        __m128i unpacked_hi = _mm_unpackhi_epi8(pixels, _mm_setzero_si128());
        
        // Simple average for demo
        __m128i avg_lo = _mm_srli_epi16(unpacked_lo, 1);
        __m128i avg_hi = _mm_srli_epi16(unpacked_hi, 1);
        
        __m128i result = _mm_packus_epi16(avg_lo, avg_hi);
        _mm_storeu_si128((__m128i*)(rgb_data + i), result);
    }

    // Process remaining pixels scalar way
    for (; i < pixel_count; i += 3) {
        uint8_t gray = (rgb_data[i] * 77 + rgb_data[i+1] * 150 + rgb_data[i+2] * 29) >> 8;
        rgb_data[i] = rgb_data[i+1] = rgb_data[i+2] = gray;
    }
}

//Brightness adjustment
void brightness_scalar(uint8_t* pixels, size_t count, int adjustment) {
    for (size_t i = 0; i < count; ++i) {
        int value = pixels[i] + adjustment;
        pixels[i] = max(0, min(255, value));
    }
}

void brightness_sse(uint8_t* pixels, size_t count, int adjustment) {
    __m128i adj_vec = _mm_set1_epi8(adjustment);
    
    size_t i = 0;
    for (; i + 15 < count; i += 16) {
        __m128i pixel_data = _mm_loadu_si128((__m128i*)(pixels + i));
        __m128i result = _mm_adds_epu8(pixel_data, adj_vec); // Saturated addition
        _mm_storeu_si128((__m128i*)(pixels + i), result);
    }

    // Process remaining pixels
    for (; i < count; ++i) {
        int value = pixels[i] + adjustment;
        pixels[i] = max(0, min(255, value));
    }
}

//Performance testing functions
void createTestData(vector<uint8_t>& src, vector<uint8_t>& dst, size_t size) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 255);
    
    src.resize(size);
    dst.resize(size);
    
    for (size_t i = 0; i < size; ++i) {
        src[i] = dis(gen);
        dst[i] = dis(gen);
    }
}

void verifyResults(const vector<uint8_t>& result1, const vector<uint8_t>& result2, const string& test_name) {
    bool match = true;
    int differences = 0;
    int max_diff = 0;
    
    for (size_t i = 0; i < result1.size() && i < result2.size(); ++i) {
        int diff = abs((int)result1[i] - (int)result2[i]);
        if (diff > 1) { // Allow for rounding differences
            match = false;
            differences++;
            max_diff = max(max_diff, diff);
        }
    }
    
    cout << test_name << " verification: ";
    if (match) {
        cout << "✓ PASSED" << endl;
    } else {
        cout << "✗ FAILED (" << differences << " differences, max: " << max_diff << ")" << endl;
    }
}

void performanceTest_AlphaBlending() {
    cout << "\n=== Alpha Blending Performance Test ===" << endl;
    
    const size_t test_size = 1024 * 1024; // 1MB of data
    const int iterations = 100;
    const uint8_t alpha = 128; // 50% transparency
    
    vector<uint8_t> src, dst_scalar, dst_sse, dst_avx2;
    createTestData(src, dst_scalar, test_size);
    
    dst_sse = dst_scalar;
    dst_avx2 = dst_scalar;
    
    // Scalar version
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        alpha_blend_scalar(dst_scalar.data(), src.data(), alpha, test_size);
    }
    auto scalar_time = high_resolution_clock::now() - start;
    
    // SSE version
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        alpha_blend_sse(dst_sse.data(), src.data(), alpha, test_size);
    }
    auto sse_time = high_resolution_clock::now() - start;
    
    // AVX2 version
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        alpha_blend_avx2(dst_avx2.data(), src.data(), alpha, test_size);
    }
    auto avx2_time = high_resolution_clock::now() - start;
    
    // Display results
    auto scalar_ms = duration_cast<milliseconds>(scalar_time).count();
    auto sse_ms = duration_cast<milliseconds>(sse_time).count();
    auto avx2_ms = duration_cast<milliseconds>(avx2_time).count();
    
    cout << "Scalar time: " << scalar_ms << " ms" << endl;
    cout << "SSE time: " << sse_ms << " ms (speedup: " << fixed << setprecision(2) << (double)scalar_ms/sse_ms << "x)" << endl;
    cout << "AVX2 time: " << avx2_ms << " ms (speedup: " << (double)scalar_ms/avx2_ms << "x)" << endl;
    
    // Verify correctness
    verifyResults(dst_scalar, dst_sse, "SSE");
    verifyResults(dst_scalar, dst_avx2, "AVX2");
}

void performanceTest_Brightness() {
    cout << "\n=== Brightness Adjustment Performance Test ===" << endl;
    
    const size_t test_size = 1024 * 1024;
    const int iterations = 200;
    const int brightness_adj = 50;
    
    vector<uint8_t> pixels_scalar, pixels_sse;
    createTestData(pixels_scalar, pixels_sse, test_size);
    
    // Scalar version
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        brightness_scalar(pixels_scalar.data(), test_size, brightness_adj);
    }
    auto scalar_time = high_resolution_clock::now() - start;
    
    // SSE version
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        brightness_sse(pixels_sse.data(), test_size, brightness_adj);
    }
    auto sse_time = high_resolution_clock::now() - start;
    
    auto scalar_ms = duration_cast<milliseconds>(scalar_time).count();
    auto sse_ms = duration_cast<milliseconds>(sse_time).count();
    
    cout << "Scalar time: " << scalar_ms << " ms" << endl;
    cout << "SSE time: " << sse_ms << " ms (speedup: " << fixed << setprecision(2) << (double)scalar_ms/sse_ms << "x)" << endl;
    
    verifyResults(pixels_scalar, pixels_sse, "Brightness SSE");
}

void demonstrateSIMDCapabilities() {
    cout << "\n=== SIMD Capabilities Analysis ===" << endl;
    
    cout << "SSE Support: ";
    #ifdef __SSE2__
        cout << "✓ SSE2 Available" << endl;
    #else
        cout << "✗ SSE2 Not Available" << endl;
    #endif
    
    cout << "AVX Support: ";
    #ifdef __AVX__
        cout << "✓ AVX Available" << endl;
    #else
        cout << "✗ AVX Not Available" << endl;
    #endif
    
    cout << "AVX2 Support: ";
    #ifdef __AVX2__
        cout << "✓ AVX2 Available" << endl;
    #else
        cout << "✗ AVX2 Not Available" << endl;
    #endif
    
    cout << "\nSIMD Register Information:" << endl;
    cout << "- SSE: 128-bit registers (XMM), processes 16 bytes at once" << endl;
    cout << "- AVX: 256-bit registers (YMM), processes 32 bytes at once" << endl;
    cout << "- Each register can hold:" << endl;
    cout << "  • 16 × 8-bit integers (bytes)" << endl;
    cout << "  • 8 × 16-bit integers (words)" << endl;
    cout << "  • 4 × 32-bit integers/floats" << endl;
    cout << "  • 2 × 64-bit integers/doubles" << endl;
}

void demonstrateMemoryAlignment() {
    cout << "\n=== Memory Alignment Demo ===" << endl;
    
    // Aligned vs unaligned memory access
    alignas(32) uint8_t aligned_data[64];
    uint8_t* unaligned_data = new uint8_t[64 + 16];
    uint8_t* unaligned_ptr = unaligned_data + 1; // Misaligned by 1 byte
    
    // Fill with test data
    for (int i = 0; i < 64; ++i) {
        aligned_data[i] = i;
        unaligned_ptr[i] = i;
    }
    
    cout << "Aligned data address: " << (void*)aligned_data 
         << " (aligned: " << ((uintptr_t)aligned_data % 32 == 0 ? "Yes" : "No") << ")" << endl;
    cout << "Unaligned data address: " << (void*)unaligned_ptr 
         << " (aligned: " << ((uintptr_t)unaligned_ptr % 32 == 0 ? "Yes" : "No") << ")" << endl;
    
    cout << "\nRecommendations:" << endl;
    cout << "• Use alignas(16) for SSE data" << endl;
    cout << "• Use alignas(32) for AVX data" << endl;
    cout << "• Use _mm_load_si128 for aligned, _mm_loadu_si128 for unaligned" << endl;
    cout << "• Aligned loads are faster but require proper data layout" << endl;
    
    delete[] unaligned_data;
}

int main(int argc, char** args) {
    cout << "=== Chapter 10: Optimizations - SIMD Instructions (SSE, AVX) ===" << endl;
    cout << "Demonstrating vectorized pixel operations for CPU graphics acceleration" << endl;
    
    demonstrateSIMDCapabilities();
    performanceTest_AlphaBlending();
    performanceTest_Brightness();
    demonstrateMemoryAlignment();
    
    cout << "\n=== SIMD Benefits in CPU Graphics ===" << endl;
    cout << "✓ Process multiple pixels simultaneously" << endl;
    cout << "✓ Significant performance improvements (2-8x speedup typical)" << endl;
    cout << "✓ Better CPU cache utilization" << endl;
    cout << "✓ Energy efficient compared to scalar operations" << endl;
    cout << "✓ Essential for real-time CPU-only graphics" << endl;
    
    cout << "\nApplications: Alpha blending, color filters, image scaling, convolution" << endl;
    
    return 0;
}