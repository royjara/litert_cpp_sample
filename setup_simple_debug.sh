#!/bin/bash
# Simple setup script for C++ debugging without complex LiteRT dependencies

set -e

echo "=== Simple C++ Debug Setup ==="
echo "Setting up debugging environment for ARM macOS (no complex dependencies)..."

# Check if we're in the right directory
if [[ ! -f "main_simple.cpp" ]]; then
    echo "Error: Please run this script from the examples/cpp_inference directory"
    exit 1
fi

echo "1. Cleaning any existing build..."
rm -rf build

echo "2. Configuring CMake build..."
cp CMakeLists_simple.txt CMakeLists.txt
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_OSX_ARCHITECTURES=arm64

echo "3. Compiling applications..."
cmake --build build --config Debug --parallel 8

echo "4. Verifying builds..."
if [[ -f "build/simple_debug_example_debug" ]]; then
    echo "✅ Simple debug example built: build/simple_debug_example_debug"
else
    echo "❌ Simple debug example build failed"
    exit 1
fi

if [[ -f "build/simple_debug_example_metal_metal_debug" ]]; then
    echo "✅ Metal debug example built: build/simple_debug_example_metal_metal_debug"
else
    echo "❌ Metal debug example build failed"
    exit 1
fi

echo "5. Setting up test model..."
if [[ ! -f "test_model.tflite" ]]; then
    echo "Downloading test model..."
    curl -L -o test_model.tflite 'https://tfhub.dev/google/lite-model/imagenet/mobilenet_v2_100_224/classification/2/metadata/1?lite-format=tflite'
fi

echo "6. Testing applications..."
echo "Testing CPU version:"
./build/simple_debug_example_debug test_model.tflite 3

echo ""
echo "Testing Metal version:"
./build/simple_debug_example_metal_metal_debug test_model.tflite 3

echo ""
echo "🎉 Simple Setup Complete!"
echo ""
echo "=== VSCode Debugging Instructions ==="
echo "1. Open VSCode in this directory:"
echo "   code ."
echo ""
echo "2. Use these debug configurations:"
echo "   - 'Debug Simple CPU' - Debug the CPU version"
echo "   - 'Debug Simple GPU (Metal)' - Debug the Metal version"
echo ""
echo "3. Key breakpoints to set in main_simple.cpp:"
echo "   - Line 45: LoadModel() - Debug model loading"
echo "   - Line 67: RunInference() - Debug computation"
echo "   - Line 75: Input processing loop"
echo "   - Line 85: CPU computation branch"
echo "   - Line 92: GPU computation branch"
echo ""
echo "4. Variables to inspect:"
echo "   - dummy_input_ - Input tensor simulation"
echo "   - dummy_output_ - Output tensor simulation"
echo "   - use_gpu_ - Acceleration mode flag"
echo "   - model_path_ - Model file path"
echo ""
echo "5. Run manually:"
echo "   ./build/simple_debug_example_debug test_model.tflite 10"
echo "   ./build/simple_debug_example_metal_metal_debug test_model.tflite 10"
echo ""
echo "=== Debugging Features ==="
echo "✅ Full debug symbols (-g -O0)"
echo "✅ ARM macOS optimization"
echo "✅ Metal framework linking"
echo "✅ VSCode C++ IntelliSense"
echo "✅ LLDB debugger integration"
echo "✅ Breakpoint support"
echo "✅ Variable inspection"
echo "✅ Step-through debugging"
echo ""
echo "This simplified example demonstrates debugging setup without"
echo "complex LiteRT dependencies, making it easier to focus on"
echo "the debugging workflow itself."
echo ""
echo "Happy debugging! 🚀"