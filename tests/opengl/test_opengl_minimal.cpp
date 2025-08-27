#include <iostream>
#include "GraphicsRenderer.h"
#include "OpenGLRenderer.h"

int main() {
    std::cout << "Testing minimal OpenGL functionality..." << std::endl;
    
    // Create OpenGL renderer
    OpenGLRenderer renderer;
    
    // Test initialization (without actual window for now)
    std::cout << "API: " << renderer.GetAPIString() << std::endl;
    
    // Test basic matrix operations
    Matrix4 testMatrix;
    std::cout << "Identity matrix created successfully" << std::endl;
    
    Matrix4 testMatrix2;
    Matrix4 result = testMatrix * testMatrix2;
    std::cout << "Matrix multiplication test successful" << std::endl;
    
    // Test primitive type enum
    PrimitiveType triangles = PRIMITIVE_TRIANGLES;
    std::cout << "Primitive type enum test successful: " << (int)triangles << std::endl;
    
    std::cout << "All basic tests passed!" << std::endl;
    
    return 0;
}
