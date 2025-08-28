#pragma once

// Real W3D type integration for OpenGL renderer
// Uses actual W3D Matrix4x4, Vector3, and Vector4 types

#ifdef ENABLE_W3D_INTEGRATION

// Include real W3D headers
#include "../../Source/WWVegas/WWMath/matrix4.h"
#include "../../Source/WWVegas/WWMath/vector3.h"
#include "../../Source/WWVegas/WWMath/vector4.h"

// Use real W3D types
using W3DMatrix4 = Matrix4x4;
using W3DVector3 = Vector3;
using W3DVector4 = Vector4;

// Utility functions to convert W3D types to OpenGL format
namespace W3DOpenGLUtils {
    
    // Convert Matrix4x4 to OpenGL float array (column-major)
    inline void MatrixToOpenGL(const Matrix4x4& w3dMatrix, float glMatrix[16]) {
        // W3D uses row-major, OpenGL uses column-major
        // W3D: Row[0] = [m00, m01, m02, m03], Row[1] = [m10, m11, m12, m13], etc.
        // OpenGL: [m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33]
        
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                glMatrix[col * 4 + row] = w3dMatrix[row][col];
            }
        }
    }
    
    // Convert OpenGL float array to Matrix4x4 (column-major to row-major)
    inline void OpenGLToMatrix(const float glMatrix[16], Matrix4x4& w3dMatrix) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                w3dMatrix[row][col] = glMatrix[col * 4 + row];
            }
        }
    }
    
    // Convert Vector3 to float array
    inline void VectorToFloat3(const Vector3& vec, float arr[3]) {
        arr[0] = vec.X;
        arr[1] = vec.Y; 
        arr[2] = vec.Z;
    }
    
    // Convert Vector4 to float array
    inline void VectorToFloat4(const Vector4& vec, float arr[4]) {
        arr[0] = vec.X;
        arr[1] = vec.Y;
        arr[2] = vec.Z;
        arr[3] = vec.W;
    }
    
    // Create Vector3 from float array
    inline Vector3 Float3ToVector(const float arr[3]) {
        return Vector3(arr[0], arr[1], arr[2]);
    }
    
    // Create Vector4 from float array  
    inline Vector4 Float4ToVector(const float arr[4]) {
        return Vector4(arr[0], arr[1], arr[2], arr[3]);
    }
    
    // Multiply two matrices using W3D operators
    inline Matrix4x4 MultiplyMatrices(const Matrix4x4& a, const Matrix4x4& b) {
        return a * b;
    }
    
    // Get matrix data as OpenGL-compatible array
    inline void GetMatrixData(const Matrix4x4& matrix, float data[16]) {
        MatrixToOpenGL(matrix, data);
    }
}

#else
    // Mock types when W3D integration is disabled - fallback implementation
    
    struct W3DMatrix4 {
        float m[16];
        W3DMatrix4() { 
            for(int i = 0; i < 16; i++) m[i] = 0.0f;
            m[0] = m[5] = m[10] = m[15] = 1.0f; // Identity
        }
        
        W3DMatrix4(const W3DMatrix4& other) {
            for(int i = 0; i < 16; i++) m[i] = other.m[i];
        }
        
        W3DMatrix4& operator=(const W3DMatrix4& other) {
            if (this != &other) {
                for(int i = 0; i < 16; i++) m[i] = other.m[i];
            }
            return *this;
        }
        
        W3DMatrix4 operator*(const W3DMatrix4& other) const {
            W3DMatrix4 result;
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    result.m[i*4 + j] = 0;
                    for(int k = 0; k < 4; k++) {
                        result.m[i*4 + j] += m[i*4 + k] * other.m[k*4 + j];
                    }
                }
            }
            return result;
        }
        
        float* data() { return m; }
        const float* data() const { return m; }
    };
    
    struct W3DVector3 {
        float x, y, z;
        W3DVector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    };
    
    struct W3DVector4 {
        float x, y, z, w;
        W3DVector4(float x = 0, float y = 0, float z = 0, float w = 0) : x(x), y(y), z(z), w(w) {}
    };
    
    // Mock utility functions for fallback mode
    namespace W3DOpenGLUtils {
        inline void MatrixToOpenGL(const W3DMatrix4& w3dMatrix, float glMatrix[16]) {
            for(int i = 0; i < 16; i++) {
                glMatrix[i] = w3dMatrix.m[i];
            }
        }
        
        inline void OpenGLToMatrix(const float glMatrix[16], W3DMatrix4& w3dMatrix) {
            for(int i = 0; i < 16; i++) {
                w3dMatrix.m[i] = glMatrix[i];
            }
        }
        
        inline void VectorToFloat3(const W3DVector3& vec, float arr[3]) {
            arr[0] = vec.x; arr[1] = vec.y; arr[2] = vec.z;
        }
        
        inline void VectorToFloat4(const W3DVector4& vec, float arr[4]) {
            arr[0] = vec.x; arr[1] = vec.y; arr[2] = vec.z; arr[3] = vec.w;
        }
        
        inline W3DMatrix4 MultiplyMatrices(const W3DMatrix4& a, const W3DMatrix4& b) {
            return a * b;
        }
        
        inline void GetMatrixData(const W3DMatrix4& matrix, float data[16]) {
            MatrixToOpenGL(matrix, data);
        }
    }
#endif
