//Chapter 9: 3D Graphics on the CPU - Basic 3D Math Library
//Standard C++ libraries
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>

//SDL3 library  
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

using namespace std;

// Book's exact Vec3 implementation
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }

    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

    Vec3 cross(const Vec3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    float length() const { return std::sqrt(x*x + y*y + z*z); }

    Vec3 normalized() const {
        float len = length();
        if (len > 0.0f) return {x/len, y/len, z/len};
        return {0, 0, 0};
    }
    
    void print() const {
        cout << "(" << fixed << setprecision(3) << x << ", " << y << ", " << z << ")";
    }
};

// Book's exact Vec4 implementation
struct Vec4 {
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}

    Vec4 operator+(const Vec4& v) const { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
    Vec4 operator-(const Vec4& v) const { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }

    float dot(const Vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    
    Vec3 xyz() const { return {x, y, z}; }
    
    // Perspective divide
    Vec3 perspectiveDivide() const {
        if (w != 0.0f) return {x/w, y/w, z/w};
        return {x, y, z};
    }
    
    void print() const {
        cout << "(" << fixed << setprecision(3) << x << ", " << y << ", " << z << ", " << w << ")";
    }
};

// Book's exact Mat4 implementation
struct Mat4 {
    float m[16]; // row-major
    
    Mat4() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    }
    
    Mat4(std::initializer_list<float> values) {
        auto it = values.begin();
        for (int i = 0; i < 16 && it != values.end(); ++i, ++it) {
            m[i] = *it;
        }
    }

    static Mat4 identity() {
        return {1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1};
    }

    static Mat4 translation(float tx, float ty, float tz) {
        return {1,0,0,tx,  0,1,0,ty,  0,0,1,tz,  0,0,0,1};
    }

    static Mat4 scale(float sx, float sy, float sz) {
        return {sx,0,0,0,  0,sy,0,0,  0,0,sz,0,  0,0,0,1};
    }

    static Mat4 rotationX(float angle) {
        float c = cosf(angle), s = sinf(angle);
        return {1,0,0,0,  0,c,-s,0,  0,s,c,0,  0,0,0,1};
    }
    
    static Mat4 rotationY(float angle) {
        float c = cosf(angle), s = sinf(angle);
        return {c,0,s,0,  0,1,0,0,  -s,0,c,0,  0,0,0,1};
    }
    
    static Mat4 rotationZ(float angle) {
        float c = cosf(angle), s = sinf(angle);
        return {c,-s,0,0,  s,c,0,0,  0,0,1,0,  0,0,0,1};
    }
    
    static Mat4 perspective(float fovy, float aspect, float near, float far) {
        float f = 1.0f / tanf(fovy * 0.5f);
        float nf = 1.0f / (near - far);
        return {
            f/aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (far + near) * nf, 2 * far * near * nf,
            0, 0, -1, 0
        };
    }
    
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 zaxis = (eye - target).normalized();
        Vec3 xaxis = up.cross(zaxis).normalized();
        Vec3 yaxis = zaxis.cross(xaxis);
        
        return {
            xaxis.x, xaxis.y, xaxis.z, -xaxis.dot(eye),
            yaxis.x, yaxis.y, yaxis.z, -yaxis.dot(eye),
            zaxis.x, zaxis.y, zaxis.z, -zaxis.dot(eye),
            0, 0, 0, 1
        };
    }

    // Multiply matrix * matrix
    Mat4 operator*(const Mat4& b) const {
        Mat4 result = {};
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                for (int k = 0; k < 4; ++k) {
                    result.m[row*4 + col] += m[row*4 + k] * b.m[k*4 + col];
                }
            }
        }
        return result;
    }

    // Multiply matrix * vector
    Vec4 operator*(const Vec4& v) const {
        return {
            m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w,
            m[4]*v.x + m[5]*v.y + m[6]*v.z + m[7]*v.w,
            m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]*v.w,
            m[12]*v.x + m[13]*v.y + m[14]*v.z + m[15]*v.w
        };
    }
    
    void print() const {
        cout << "Matrix 4x4:" << endl;
        for (int row = 0; row < 4; ++row) {
            cout << "  ";
            for (int col = 0; col < 4; ++col) {
                cout << fixed << setprecision(3) << setw(8) << m[row*4 + col];
            }
            cout << endl;
        }
    }
};

// Book's data structures for 3D rendering
struct Edge {
    int start, end;
};

struct Triangle {
    int vertices[3];
    Vec3 normal;
};

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Edge> edges;
};

// Helper function to create a simple cube mesh
Mesh createCubeMesh() {
    Mesh cube;
    
    // Cube vertices
    cube.vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, // Front face
        {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}  // Back face
    };
    
    // Cube triangles (2 per face)
    cube.triangles = {
        {{0,1,2}, {0,0,-1}}, {{0,2,3}, {0,0,-1}}, // Front
        {{5,4,7}, {0,0,1}},  {{5,7,6}, {0,0,1}},  // Back
        {{4,0,3}, {-1,0,0}}, {{4,3,7}, {-1,0,0}}, // Left
        {{1,5,6}, {1,0,0}},  {{1,6,2}, {1,0,0}},  // Right
        {{3,2,6}, {0,1,0}},  {{3,6,7}, {0,1,0}},  // Top
        {{4,5,1}, {0,-1,0}}, {{4,1,0}, {0,-1,0}}  // Bottom
    };
    
    return cube;
}

// Simple viewport transformation
Vec3 viewportTransform(const Vec3& ndc, int width, int height) {
    return {
        (ndc.x + 1.0f) * 0.5f * width,
        (1.0f - ndc.y) * 0.5f * height,  // Flip Y
        ndc.z
    };
}

// Demonstration functions
void demonstrateVectorOperations() {
    cout << "\n=== Vector Operations Demo ===" << endl;
    
    Vec3 a(1, 2, 3);
    Vec3 b(4, 5, 6);
    
    cout << "Vector a: "; a.print(); cout << endl;
    cout << "Vector b: "; b.print(); cout << endl;
    
    Vec3 sum = a + b;
    cout << "a + b: "; sum.print(); cout << endl;
    
    Vec3 diff = a - b;
    cout << "a - b: "; diff.print(); cout << endl;
    
    float dotProduct = a.dot(b);
    cout << "a · b: " << dotProduct << endl;
    
    Vec3 crossProduct = a.cross(b);
    cout << "a × b: "; crossProduct.print(); cout << endl;
    
    cout << "Length of a: " << a.length() << endl;
    
    Vec3 normalized = a.normalized();
    cout << "Normalized a: "; normalized.print(); cout << endl;
}

void demonstrateMatrixOperations() {
    cout << "\n=== Matrix Operations Demo ===" << endl;
    
    Mat4 identity = Mat4::identity();
    cout << "Identity Matrix:" << endl;
    identity.print();
    
    Mat4 translation = Mat4::translation(2, 3, 4);
    cout << "\nTranslation(2,3,4):" << endl;
    translation.print();
    
    Mat4 rotation = Mat4::rotationY(M_PI / 4);
    cout << "\nRotation Y (45°):" << endl;
    rotation.print();
    
    Mat4 scale = Mat4::scale(2, 2, 2);
    cout << "\nScale(2,2,2):" << endl;
    scale.print();
    
    // Combined transformation
    Mat4 combined = translation * rotation * scale;
    cout << "\nCombined (T * R * S):" << endl;
    combined.print();
}

void demonstrate3DPipeline() {
    cout << "\n=== 3D Transformation Pipeline Demo ===" << endl;
    
    // Create a simple vertex
    Vec3 vertex(1, 1, 1);
    cout << "Original vertex: "; vertex.print(); cout << endl;
    
    // Model transformation
    Mat4 model = Mat4::rotationY(M_PI / 6) * Mat4::scale(2, 2, 2);
    Vec4 worldPos = model * Vec4(vertex);
    cout << "After model transform: "; worldPos.print(); cout << endl;
    
    // View transformation
    Vec3 eye(0, 0, 5);
    Vec3 target(0, 0, 0);
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, target, up);
    Vec4 viewPos = view * worldPos;
    cout << "After view transform: "; viewPos.print(); cout << endl;
    
    // Projection transformation
    Mat4 proj = Mat4::perspective(M_PI / 4, 16.0f/9.0f, 0.1f, 100.0f);
    Vec4 clipPos = proj * viewPos;
    cout << "After projection: "; clipPos.print(); cout << endl;
    
    // Perspective divide
    Vec3 ndcPos = clipPos.perspectiveDivide();
    cout << "After perspective divide (NDC): "; ndcPos.print(); cout << endl;
    
    // Viewport transformation
    Vec3 screenPos = viewportTransform(ndcPos, 800, 600);
    cout << "Final screen position: "; screenPos.print(); cout << endl;
    
    // MVP matrix  
    cout << "\n=== MVP Matrix Composition ===" << endl;
    Mat4 mvp = proj * view * model;
    cout << "MVP Matrix:" << endl;
    mvp.print();
    
    // Direct transformation with MVP
    Vec4 finalPos = mvp * Vec4(vertex);
    cout << "\nDirect MVP transformation: "; finalPos.print(); cout << endl;
    Vec3 finalNDC = finalPos.perspectiveDivide();
    cout << "Final NDC (should match above): "; finalNDC.print(); cout << endl;
}

void demonstrateMeshTransformation() {
    cout << "\n=== Mesh Transformation Demo ===" << endl;
    
    Mesh cube = createCubeMesh();
    cout << "Created cube with " << cube.vertices.size() << " vertices and " 
         << cube.triangles.size() << " triangles" << endl;
    
    // Transform all vertices
    Mat4 mvp = Mat4::perspective(M_PI/4, 1.0f, 0.1f, 100.0f) * 
               Mat4::lookAt({3,3,3}, {0,0,0}, {0,1,0}) *
               Mat4::rotationY(M_PI/4);
    
    cout << "\nTransformed vertices (screen space):" << endl;
    for (size_t i = 0; i < cube.vertices.size(); ++i) {
        Vec4 transformed = mvp * Vec4(cube.vertices[i]);
        Vec3 ndc = transformed.perspectiveDivide();
        Vec3 screen = viewportTransform(ndc, 800, 600);
        
        cout << "Vertex " << i << ": ";
        cube.vertices[i].print();
        cout << " -> ";
        screen.print();
        cout << endl;
    }
}

int main(int argc, char** args) {
    cout << "=== Chapter 9: 3D Graphics on the CPU - Basic 3D Math ===" << endl;
    cout << "Demonstrating vectors, matrices, and transformations" << endl;
    
    demonstrateVectorOperations();
    demonstrateMatrixOperations();
    demonstrate3DPipeline();
    demonstrateMeshTransformation();
    
    cout << "\n=== Real-World Application Notes ===" << endl;
    cout << "This math library enables:" << endl;
    cout << "- Object positioning and orientation in 3D space" << endl;
    cout << "- Camera and viewport control" << endl;  
    cout << "- Perspective and orthographic projection" << endl;
    cout << "- Vertex transformation pipeline" << endl;
    cout << "- Mesh manipulation and rendering" << endl;
    cout << "\nNext steps: Triangle rasterization and lighting calculations" << endl;
    
    return 0;
}