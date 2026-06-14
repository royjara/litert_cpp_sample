#!/bin/bash
# Setup script for LiteRT C++ debugging environment

set -e

echo "=== LiteRT C++ Debug Setup ==="
echo "Setting up debugging environment for ARM macOS..."

# Check if we're in the right directory
if [[ ! -f "main.cpp" ]]; then
    echo "Error: Please run this script from the examples/cpp_inference directory"
    exit 1
fi

# Set up environment
export HERMETIC_PYTHON_VERSION=3.11

echo "1. Building LiteRT dependencies with Bazel..."
cd ../..
bazel build //litert/cc:litert_compiled_model --config=macos_arm64
bazel build //litert/c:litert_c_api --config=macos_arm64

echo "2. Building C++ example application..."
cd examples/cpp_inference

# Clean any existing build
rm -rf build

# Configure CMake
echo "3. Configuring CMake build..."
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_OSX_ARCHITECTURES=arm64

# Build
echo "4. Compiling application..."
cmake --build build --config Debug --parallel 8

echo "5. Verifying build..."
if [[ -f "build/litert_inference_example" ]]; then
    echo "✅ Build successful! Executable created: build/litert_inference_example"
else
    echo "❌ Build failed - executable not found"
    exit 1
fi

echo "6. Testing with model..."
if [[ -f "test_model.tflite" ]]; then
    echo "✅ Test model found"
    echo "Running quick test..."
    ./build/litert_inference_example test_model.tflite 3
else
    echo "⚠️  Test model not found. Downloading..."
    curl -L -o test_model.tflite 'https://tfhub.dev/google/lite-model/imagenet/mobilenet_v2_100_224/classification/2/metadata/1?lite-format=tflite'
    echo "✅ Model downloaded. Running test..."
    ./build/litert_inference_example test_model.tflite 3
fi

echo ""
echo "🎉 Setup Complete!"
echo ""
echo "=== Usage Instructions ==="
echo "1. Open VSCode in this directory:"
echo "   code ."
echo ""
echo "2. Use these debug configurations:"
echo "   - Debug LiteRT CPU"
echo "   - Debug LiteRT GPU (Metal)"
echo ""
echo "3. Set breakpoints in main.cpp at:"
echo "   - Line 54: Model loading"
echo "   - Line 195: Inference execution"
echo "   - Line 236: Tensor buffer access"
echo ""
echo "4. Run manually:"
echo "   ./build/litert_inference_example test_model.tflite 10"
echo ""
echo "=== Performance Comparison ==="
echo "The app will automatically benchmark both CPU and GPU inference."
echo "Expected results on Apple Silicon:"
echo "- CPU: ~2-3ms per inference"
echo "- GPU (Metal): ~1-2ms per inference"
echo ""
echo "Happy debugging! 🚀"