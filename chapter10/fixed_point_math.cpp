//Chapter 10: Optimizations - Fixed-Point Math (Q16.16)
//Standard C++ libraries
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <vector>
#include <cstdint>

using namespace std;
using namespace std::chrono;

// Book's exact Q16.16 fixed-point implementation
typedef int32_t Fixed16_16;

const int FIXED_POINT_SHIFT = 16;
const Fixed16_16 FIXED_POINT_ONE = 1 << FIXED_POINT_SHIFT;

// Convert from floating point to fixed-point
Fixed16_16 floatToFixed(float f) {
    return (Fixed16_16)(f * FIXED_POINT_ONE);
}

// Convert from fixed-point to floating point
float fixedToFloat(Fixed16_16 f) {
    return (float)f / FIXED_POINT_ONE;
}

// Convert from integer to fixed-point
Fixed16_16 intToFixed(int i) {
    return i << FIXED_POINT_SHIFT;
}

// Convert from fixed-point to integer
int fixedToInt(Fixed16_16 f) {
    return f >> FIXED_POINT_SHIFT;
}

// Book's exact fixed-point operations
Fixed16_16 fixed_add(Fixed16_16 a, Fixed16_16 b) {
    return a + b;
}

Fixed16_16 fixed_sub(Fixed16_16 a, Fixed16_16 b) {
    return a - b;
}

Fixed16_16 fixed_mul(Fixed16_16 a, Fixed16_16 b) {
    int64_t temp = (int64_t)a * (int64_t)b; // 64-bit to avoid overflow
    return (Fixed16_16)(temp >> FIXED_POINT_SHIFT);            // Shift right by fractional bits
}

Fixed16_16 fixed_div(Fixed16_16 a, Fixed16_16 b) {
    int64_t temp = ((int64_t)a << FIXED_POINT_SHIFT) / b;
    return (Fixed16_16)temp;
}

// Fixed-point trigonometry (simplified versions)
Fixed16_16 fixed_sin(Fixed16_16 angle) {
    float angleFloat = fixedToFloat(angle);
    float result = sinf(angleFloat);
    return floatToFixed(result);
}

Fixed16_16 fixed_cos(Fixed16_16 angle) {
    float angleFloat = fixedToFloat(angle);
    float result = cosf(angleFloat);
    return floatToFixed(result);
}

Fixed16_16 fixed_sqrt(Fixed16_16 x) {
    if (x <= 0) return 0;
    
    // Newton's method for fixed-point square root
    Fixed16_16 guess = x >> 1;
    Fixed16_16 prev;
    
    for (int i = 0; i < 10; ++i) {
        prev = guess;
        guess = (guess + fixed_div(x, guess)) >> 1;
        if (abs(guess - prev) < 2) break;
    }
    
    return guess;
}

// Fixed-point 2D vector
struct FixedVec2 {
    Fixed16_16 x, y;
    
    FixedVec2() : x(0), y(0) {}
    FixedVec2(Fixed16_16 x, Fixed16_16 y) : x(x), y(y) {}
    FixedVec2(float fx, float fy) : x(floatToFixed(fx)), y(floatToFixed(fy)) {}
    
    FixedVec2 operator+(const FixedVec2& v) const {
        return {fixed_add(x, v.x), fixed_add(y, v.y)};
    }
    
    FixedVec2 operator-(const FixedVec2& v) const {
        return {fixed_sub(x, v.x), fixed_sub(y, v.y)};
    }
    
    FixedVec2 operator*(Fixed16_16 scalar) const {
        return {fixed_mul(x, scalar), fixed_mul(y, scalar)};
    }
    
    Fixed16_16 dot(const FixedVec2& v) const {
        return fixed_add(fixed_mul(x, v.x), fixed_mul(y, v.y));
    }
    
    Fixed16_16 length() const {
        Fixed16_16 lengthSquared = fixed_add(fixed_mul(x, x), fixed_mul(y, y));
        return fixed_sqrt(lengthSquared);
    }
    
    FixedVec2 normalized() const {
        Fixed16_16 len = length();
        if (len > 0) {
            return {fixed_div(x, len), fixed_div(y, len)};
        }
        return {0, 0};
    }
    
    void print() const {
        cout << "(" << fixed << setprecision(4) << fixedToFloat(x) 
             << ", " << fixedToFloat(y) << ")";
    }
};

// Performance comparison functions
void demonstrateBasicOperations() {
    cout << "\n=== Fixed-Point Basic Operations Demo ===" << endl;
    
    Fixed16_16 a = floatToFixed(3.14159f);
    Fixed16_16 b = floatToFixed(2.71828f);
    
    cout << "a (π): " << fixedToFloat(a) << " (raw: 0x" << hex << a << dec << ")" << endl;
    cout << "b (e): " << fixedToFloat(b) << " (raw: 0x" << hex << b << dec << ")" << endl;
    
    Fixed16_16 sum = fixed_add(a, b);
    cout << "a + b: " << fixedToFloat(sum) << endl;
    
    Fixed16_16 diff = fixed_sub(a, b);
    cout << "a - b: " << fixedToFloat(diff) << endl;
    
    Fixed16_16 product = fixed_mul(a, b);
    cout << "a * b: " << fixedToFloat(product) << endl;
    
    Fixed16_16 quotient = fixed_div(a, b);
    cout << "a / b: " << fixedToFloat(quotient) << endl;
    
    Fixed16_16 sqrt_a = fixed_sqrt(a);
    cout << "sqrt(a): " << fixedToFloat(sqrt_a) << endl;
    
    // Compare with floating point
    cout << "\nFloating-point comparison:" << endl;
    float fa = 3.14159f, fb = 2.71828f;
    cout << "Float a * b: " << (fa * fb) << endl;
    cout << "Float a / b: " << (fa / fb) << endl;
    cout << "Float sqrt(a): " << sqrtf(fa) << endl;
}

void demonstrateVectorOperations() {
    cout << "\n=== Fixed-Point Vector Operations Demo ===" << endl;
    
    FixedVec2 v1(3.0f, 4.0f);
    FixedVec2 v2(1.0f, 2.0f);
    
    cout << "Vector v1: "; v1.print(); cout << endl;
    cout << "Vector v2: "; v2.print(); cout << endl;
    
    FixedVec2 sum = v1 + v2;
    cout << "v1 + v2: "; sum.print(); cout << endl;
    
    FixedVec2 diff = v1 - v2;
    cout << "v1 - v2: "; diff.print(); cout << endl;
    
    Fixed16_16 scalar = floatToFixed(2.5f);
    FixedVec2 scaled = v1 * scalar;
    cout << "v1 * 2.5: "; scaled.print(); cout << endl;
    
    Fixed16_16 dotProduct = v1.dot(v2);
    cout << "v1 · v2: " << fixedToFloat(dotProduct) << endl;
    
    Fixed16_16 length = v1.length();
    cout << "Length of v1: " << fixedToFloat(length) << endl;
    
    FixedVec2 normalized = v1.normalized();
    cout << "Normalized v1: "; normalized.print(); cout << endl;
}

void performanceComparison() {
    cout << "\n=== Performance Comparison: Fixed vs Float ===" << endl;
    
    const int iterations = 1000000;
    vector<float> floatResults(iterations);
    vector<Fixed16_16> fixedResults(iterations);
    
    // Fixed-point performance test
    auto start = high_resolution_clock::now();
    
    Fixed16_16 fixedA = floatToFixed(1.5f);
    Fixed16_16 fixedB = floatToFixed(2.3f);
    
    for (int i = 0; i < iterations; ++i) {
        Fixed16_16 result = fixed_mul(fixedA, fixedB);
        result = fixed_add(result, intToFixed(i));
        result = fixed_div(result, floatToFixed(1.1f));
        fixedResults[i] = result;
    }
    
    auto fixedTime = high_resolution_clock::now() - start;
    
    // Floating-point performance test  
    start = high_resolution_clock::now();
    
    float floatA = 1.5f;
    float floatB = 2.3f;
    
    for (int i = 0; i < iterations; ++i) {
        float result = floatA * floatB;
        result = result + (float)i;
        result = result / 1.1f;
        floatResults[i] = result;
    }
    
    auto floatTime = high_resolution_clock::now() - start;
    
    cout << "Fixed-point time: " << duration_cast<microseconds>(fixedTime).count() << " μs" << endl;
    cout << "Floating-point time: " << duration_cast<microseconds>(floatTime).count() << " μs" << endl;
    
    float speedup = (float)duration_cast<microseconds>(floatTime).count() / 
                    duration_cast<microseconds>(fixedTime).count();
    
    if (speedup > 1.0f) {
        cout << "Fixed-point is " << speedup << "x faster" << endl;
    } else {
        cout << "Floating-point is " << (1.0f/speedup) << "x faster" << endl;
    }
    
    // Verify results are approximately equal
    float maxError = 0.0f;
    for (int i = 0; i < min(10, iterations); ++i) {
        float fixedAsFloat = fixedToFloat(fixedResults[i]);
        float error = abs(fixedAsFloat - floatResults[i]);
        maxError = max(maxError, error);
    }
    cout << "Maximum error in first 10 results: " << maxError << endl;
}

void demonstrateGraphicsApplications() {
    cout << "\n=== Graphics Applications Demo ===" << endl;
    
    // 2D rotation using fixed-point
    cout << "2D Rotation using fixed-point trigonometry:" << endl;
    
    FixedVec2 point(5.0f, 0.0f);
    Fixed16_16 angle = floatToFixed(M_PI / 4); // 45 degrees
    
    cout << "Original point: "; point.print(); cout << endl;
    cout << "Rotation angle: " << fixedToFloat(angle) << " radians (45°)" << endl;
    
    Fixed16_16 cosAngle = fixed_cos(angle);
    Fixed16_16 sinAngle = fixed_sin(angle);
    
    // Rotation matrix application: [cos -sin; sin cos] * [x; y]
    Fixed16_16 newX = fixed_sub(fixed_mul(cosAngle, point.x), fixed_mul(sinAngle, point.y));
    Fixed16_16 newY = fixed_add(fixed_mul(sinAngle, point.x), fixed_mul(cosAngle, point.y));
    
    FixedVec2 rotatedPoint(newX, newY);
    cout << "Rotated point: "; rotatedPoint.print(); cout << endl;
    
    // Linear interpolation
    cout << "\nLinear interpolation between two points:" << endl;
    FixedVec2 p1(0.0f, 0.0f);
    FixedVec2 p2(10.0f, 10.0f);
    Fixed16_16 t = floatToFixed(0.3f); // 30% interpolation
    
    FixedVec2 diff = p2 - p1;
    FixedVec2 interpolated = p1 + (diff * t);
    
    cout << "Point 1: "; p1.print(); cout << endl;
    cout << "Point 2: "; p2.print(); cout << endl;
    cout << "Interpolation factor: " << fixedToFloat(t) << endl;
    cout << "Interpolated point: "; interpolated.print(); cout << endl;
}

void demonstratePrecisionAnalysis() {
    cout << "\n=== Precision Analysis ===" << endl;
    
    cout << "Q16.16 Fixed-Point Format Characteristics:" << endl;
    cout << "- Total bits: 32" << endl;
    cout << "- Integer bits: 16" << endl;
    cout << "- Fractional bits: 16" << endl;
    cout << "- Range: " << fixedToFloat(INT32_MIN) << " to " << fixedToFloat(INT32_MAX) << endl;
    cout << "- Resolution: " << (1.0f / FIXED_POINT_ONE) << endl;
    cout << "- One (1.0): 0x" << hex << FIXED_POINT_ONE << dec << endl;
    
    cout << "\nPrecision test with small values:" << endl;
    float values[] = {0.1f, 0.01f, 0.001f, 0.0001f};
    
    for (float val : values) {
        Fixed16_16 fixed = floatToFixed(val);
        float converted = fixedToFloat(fixed);
        float error = abs(val - converted);
        
        cout << "Original: " << val << ", Fixed: " << converted 
             << ", Error: " << error << endl;
    }
}

int main(int argc, char** args) {
    cout << "=== Chapter 10: Optimizations - Fixed-Point Math (Q16.16) ===" << endl;
    cout << "Demonstrating deterministic integer-based arithmetic for CPU graphics" << endl;
    
    demonstrateBasicOperations();
    demonstrateVectorOperations();
    performanceComparison();
    demonstrateGraphicsApplications();
    demonstratePrecisionAnalysis();
    
    cout << "\n=== Benefits of Fixed-Point in CPU Graphics ===" << endl;
    cout << "✓ Deterministic performance across platforms" << endl;
    cout << "✓ Lower power consumption than floating-point" << endl;
    cout << "✓ Works on CPUs without FPUs" << endl;
    cout << "✓ Avoids floating-point rounding inconsistencies" << endl;
    cout << "✓ Suitable for embedded and legacy systems" << endl;
    cout << "\nApplications: 2D transformations, interpolation, collision detection" << endl;
    
    return 0;
}