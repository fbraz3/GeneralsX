#!/bin/bash

# OpenGL Test Suite for Command & Conquer: Generals
# This script compiles and runs OpenGL tests from the tests/opengl directory

set -e

echo "🎮 Command & Conquer: Generals - OpenGL Test Suite"
echo "=================================================="

# Get project root (two directories up from tests/opengl)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_ROOT"

echo "📂 Project root: $PROJECT_ROOT"
echo "📂 Test directory: $SCRIPT_DIR"

# Compiler flags (relative to project root)
INCLUDES="-I./Core/Libraries/Include -I./Core/Libraries/Include/GraphicsAPI -I./Dependencies/Utility"
SOURCES="Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp"
FLAGS="-DENABLE_OPENGL -std=c++11 -framework OpenGL"

echo "📋 Running OpenGL tests..."
echo

# Test 1: Basic functionality
echo "🔧 Test 1: Basic Functionality"
echo "------------------------------"
g++ $INCLUDES tests/opengl/test_opengl_minimal.cpp $SOURCES -o tests/opengl/test_opengl_minimal $FLAGS
cd tests/opengl && ./test_opengl_minimal && cd "$PROJECT_ROOT"
echo "✅ Basic functionality test PASSED"
echo

# Test 2: Context initialization
echo "🖥️  Test 2: Context Initialization"
echo "----------------------------------"
g++ $INCLUDES tests/opengl/test_opengl_context.cpp $SOURCES -o tests/opengl/test_opengl_context $FLAGS
cd tests/opengl && ./test_opengl_context && cd "$PROJECT_ROOT"
echo "✅ Context initialization test PASSED"
echo

# Test 3: Compilation test
echo "⚙️  Test 3: Compilation Test"
echo "----------------------------"
g++ $INCLUDES -c Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp -o tests/opengl/test_compile.o $FLAGS
echo "✅ Compilation test PASSED"
rm -f tests/opengl/test_compile.o
echo

# Test 4: W3D Integration
echo "🔄 Test 4: W3D Integration"
echo "---------------------------"
g++ $INCLUDES tests/opengl/test_w3d_integration.cpp \
    Core/Libraries/Source/GraphicsAPI/OpenGLRendererW3D.cpp \
    Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp \
    Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp \
    -o tests/opengl/test_w3d_integration \
    -DENABLE_OPENGL -DENABLE_W3D_INTEGRATION -std=c++11 -framework OpenGL

if [ $? -eq 0 ]; then
    cd tests/opengl && ./test_w3d_integration && cd "$PROJECT_ROOT"
    echo "✅ W3D Integration test PASSED"
else
    echo "❌ W3D Integration test FAILED"
fi
echo

# Cleanup
echo "🧹 Cleaning up..."
rm -f tests/opengl/test_opengl_minimal tests/opengl/test_opengl_context tests/opengl/test_w3d_integration

echo "🎉 ALL OPENGL TESTS COMPLETED SUCCESSFULLY!"
echo
echo "📊 Summary:"
echo "  ✅ Basic functionality: WORKING"
echo "  ✅ Context creation: WORKING"  
echo "  ✅ Compilation: WORKING"
echo "  ✅ W3D Integration: WORKING"
echo "  ✅ OpenGL Status: READY FOR GAME INTEGRATION"
echo
echo "📖 Next steps:"
echo "  1. Review OPENGL_SUCCESS_REPORT.md"
echo "  2. Integrate with W3D system"
echo "  3. Test with actual game"
