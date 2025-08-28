#pragma once

#ifndef W3D_TYPES_MINIMAL_H
#define W3D_TYPES_MINIMAL_H

// Minimal W3D types for testing without full engine dependencies
// This provides a simplified interface for OpenGL integration testing

#include <cstdint>
#include <cmath>
#include <cstring>

// Forward declarations to avoid complex dependencies
namespace W3DMinimal {

// Simplified Matrix4x4 compatible with W3D structure
struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        // Initialize to identity
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }
    
    Matrix4x4(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    
    float* operator[](int row) { return m[row]; }
    const float* operator[](int row) const { return m[row]; }
};

// Simplified Vector3 compatible with W3D structure
struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    Vector3 cross(const Vector3& other) const {
        return Vector3(y * other.z - z * other.y,
                      z * other.x - x * other.z,
                      x * other.y - y * other.x);
    }
    
    float length() const {
        return sqrtf(x * x + y * y + z * z);
    }
    
    Vector3 normalize() const {
        float len = length();
        if (len > 0.0f) {
            return Vector3(x / len, y / len, z / len);
        }
        return Vector3();
    }
};

// Simplified Vector4 compatible with W3D structure
struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vector4(const Vector3& v3, float w_) : x(v3.x), y(v3.y), z(v3.z), w(w_) {}
    
    Vector4 operator+(const Vector4& other) const {
        return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    
    Vector4 operator-(const Vector4& other) const {
        return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    
    Vector4 operator*(float scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    
    float dot(const Vector4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
};

} // namespace W3DMinimal

// Type aliases for compatibility - only define if not already defined
#ifndef W3D_MATRIX4_DEFINED
#define W3D_MATRIX4_DEFINED
using W3DMatrix4 = W3DMinimal::Matrix4x4;
#endif

#ifndef W3D_VECTOR3_DEFINED
#define W3D_VECTOR3_DEFINED
using W3DVector3 = W3DMinimal::Vector3;
#endif

#ifndef W3D_VECTOR4_DEFINED
#define W3D_VECTOR4_DEFINED
using W3DVector4 = W3DMinimal::Vector4;
#endif

// Utility functions to convert W3D types to OpenGL format
namespace W3DOpenGLUtils {
    
    // Convert Matrix4x4 to OpenGL float array (column-major)
    inline void MatrixToOpenGL(const W3DMatrix4& w3dMatrix, float glMatrix[16]) {
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
    inline void OpenGLToMatrix(const float glMatrix[16], W3DMatrix4& w3dMatrix) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                w3dMatrix[row][col] = glMatrix[col * 4 + row];
            }
        }
    }
    
    // Convert Vector3 to OpenGL float array
    inline void Vector3ToOpenGL(const W3DVector3& w3dVector, float glVector[3]) {
        glVector[0] = w3dVector.x;
        glVector[1] = w3dVector.y;
        glVector[2] = w3dVector.z;
    }
    
    // Convert Vector4 to OpenGL float array
    inline void Vector4ToOpenGL(const W3DVector4& w3dVector, float glVector[4]) {
        glVector[0] = w3dVector.x;
        glVector[1] = w3dVector.y;
        glVector[2] = w3dVector.z;
        glVector[3] = w3dVector.w;
    }
    
    // Convert OpenGL float array to Vector3
    inline void OpenGLToVector3(const float glVector[3], W3DVector3& w3dVector) {
        w3dVector.x = glVector[0];
        w3dVector.y = glVector[1];
        w3dVector.z = glVector[2];
    }
    
    // Convert OpenGL float array to Vector4
    inline void OpenGLToVector4(const float glVector[4], W3DVector4& w3dVector) {
        w3dVector.x = glVector[0];
        w3dVector.y = glVector[1];
        w3dVector.z = glVector[2];
        w3dVector.w = glVector[3];
    }
    
    // Helper to create perspective projection matrix
    inline W3DMatrix4 CreatePerspectiveMatrix(float fov, float aspect, float nearPlane, float farPlane) {
        float f = 1.0f / tanf(fov * 0.5f);
        float nf = 1.0f / (nearPlane - farPlane);
        
        return W3DMatrix4(
            f / aspect, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, (farPlane + nearPlane) * nf, (2.0f * farPlane * nearPlane) * nf,
            0.0f, 0.0f, -1.0f, 0.0f
        );
    }
    
    // Helper to create view matrix
    inline W3DMatrix4 CreateViewMatrix(const W3DVector3& eye, const W3DVector3& center, const W3DVector3& up) {
        W3DVector3 f = (center - eye).normalize();
        W3DVector3 s = f.cross(up).normalize();
        W3DVector3 u = s.cross(f);
        
        return W3DMatrix4(
            s.x, u.x, -f.x, 0.0f,
            s.y, u.y, -f.y, 0.0f,
            s.z, u.z, -f.z, 0.0f,
            -s.dot(eye), -u.dot(eye), f.dot(eye), 1.0f
        );
    }
    
    // Helper to multiply matrices
    inline W3DMatrix4 MultiplyMatrices(const W3DMatrix4& a, const W3DMatrix4& b) {
        W3DMatrix4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result[i][j] += a[i][k] * b[k][j];
                }
            }
        }
        return result;
    }
}

#endif // W3D_TYPES_MINIMAL_H
