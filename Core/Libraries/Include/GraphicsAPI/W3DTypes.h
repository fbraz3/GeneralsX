#pragma once

// W3D Integration Header
// This file bridges our OpenGL renderer with W3D-compatible types
// Simplified version to avoid complex dependencies

// Forward declarations to avoid including full W3D headers
namespace W3DCompat {
    
    // Simple Matrix4x4 class compatible with W3D structure
    class Matrix4x4 {
    public:
        float Row[4][4];
        
        Matrix4x4() {
            Make_Identity();
        }
        
        void Make_Identity() {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    Row[i][j] = (i == j) ? 1.0f : 0.0f;
                }
            }
        }
        
        // Array access operator
        float* operator[](int i) { return Row[i]; }
        const float* operator[](int i) const { return Row[i]; }
        
        // Multiplication operator
        Matrix4x4 operator*(const Matrix4x4& other) const {
            Matrix4x4 result;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result.Row[i][j] = 0;
                    for (int k = 0; k < 4; k++) {
                        result.Row[i][j] += Row[i][k] * other.Row[k][j];
                    }
                }
            }
            return result;
        }
        
        // Data access for OpenGL
        const float* data() const { return &Row[0][0]; }
    };
    
    // Simple Vector3 class
    class Vector3 {
    public:
        float X, Y, Z;
        
        Vector3() : X(0), Y(0), Z(0) {}
        Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
        
        float operator[](int i) const {
            switch(i) {
                case 0: return X;
                case 1: return Y;
                case 2: return Z;
                default: return 0;
            }
        }
    };
    
    // Simple Vector4 class
    class Vector4 {
    public:
        float X, Y, Z, W;
        
        Vector4() : X(0), Y(0), Z(0), W(0) {}
        Vector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}
        
        float operator[](int i) const {
            switch(i) {
                case 0: return X;
                case 1: return Y;
                case 2: return Z;
                case 3: return W;
                default: return 0;
            }
        }
    };
    
    // Static matrix multiplication function (W3D compatible)
    static void Multiply(const Matrix4x4& A, const Matrix4x4& B, Matrix4x4* result) {
        *result = A * B;
    }
}

// Define our conversion utilities
namespace W3DOpenGLBridge {
    
    // Convert W3D Matrix4x4 to OpenGL-compatible format
    inline void ConvertMatrix(const W3DCompat::Matrix4x4& w3dMatrix, float* openglMatrix) {
        // W3D uses row-major, OpenGL expects column-major
        // We need to transpose the matrix
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                openglMatrix[col * 4 + row] = w3dMatrix[row][col];
            }
        }
    }
    
    // Convert OpenGL matrix back to W3D format
    inline void ConvertMatrixFromOpenGL(const float* openglMatrix, W3DCompat::Matrix4x4& w3dMatrix) {
        // Transpose back from column-major to row-major
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                w3dMatrix[row][col] = openglMatrix[col * 4 + row];
            }
        }
    }
    
    // Convert W3D Vector3 to OpenGL-compatible format
    inline void ConvertVector3(const W3DCompat::Vector3& w3dVector, float* openglVector) {
        openglVector[0] = w3dVector.X;
        openglVector[1] = w3dVector.Y;
        openglVector[2] = w3dVector.Z;
    }
    
    // Convert W3D Vector4 to OpenGL-compatible format
    inline void ConvertVector4(const W3DCompat::Vector4& w3dVector, float* openglVector) {
        openglVector[0] = w3dVector.X;
        openglVector[1] = w3dVector.Y;
        openglVector[2] = w3dVector.Z;
        openglVector[3] = w3dVector.W;
    }
}

// Type aliases for easier integration
using W3DMatrix4 = W3DCompat::Matrix4x4;
using W3DMatrix3 = W3DCompat::Matrix4x4; // Simplified, can be Matrix3D later
using W3DVector3 = W3DCompat::Vector3;
using W3DVector4 = W3DCompat::Vector4;
