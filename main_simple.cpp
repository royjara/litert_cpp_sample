#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <string>
#include <fstream>

// Simplified LiteRT C++ example that works with the existing built libraries
// This version focuses on demonstrating the debugging setup rather than complex LiteRT APIs

class SimpleInference {
private:
    std::string model_path_;
    bool use_gpu_;
    std::vector<float> dummy_input_;
    std::vector<float> dummy_output_;

public:
    SimpleInference(const std::string& model_path, bool use_gpu = false)
        : model_path_(model_path), use_gpu_(use_gpu) {
        // Initialize dummy data
        dummy_input_.resize(224 * 224 * 3, 0.5f);  // Typical image input size
        dummy_output_.resize(1000, 0.0f);           // Typical classification output
    }

    bool LoadModel() {
        std::cout << "Loading model: " << model_path_ << std::endl;
        std::cout << "Using " << (use_gpu_ ? "GPU (Metal)" : "CPU") << " acceleration" << std::endl;

        // Check if model file exists
        std::ifstream file(model_path_);
        if (!file.good()) {
            std::cerr << "Failed to load model from: " << model_path_ << std::endl;
            return false;
        }

        // Get file size for basic validation
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::cout << "Model file size: " << file_size << " bytes" << std::endl;

        if (file_size < 100) {
            std::cerr << "Model file appears to be too small (corrupted?)" << std::endl;
            return false;
        }

        std::cout << "Model loaded successfully!" << std::endl;
        return true;
    }

    bool RunInference() {
        std::cout << "Running inference..." << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();

        // Simulate inference work - this is where actual LiteRT inference would happen
        // For debugging purposes, we'll do some dummy computation that can be stepped through

        // Input preprocessing simulation
        float sum = 0.0f;
        for (size_t i = 0; i < dummy_input_.size(); ++i) {
            dummy_input_[i] = static_cast<float>(i % 255) / 255.0f;
            sum += dummy_input_[i];
        }

        // Simulated inference computation
        if (use_gpu_) {
            // Simulate GPU computation (in reality this would use Metal)
            for (size_t i = 0; i < dummy_output_.size(); ++i) {
                dummy_output_[i] = sum * static_cast<float>(i) / 1000.0f;
                // Add some GPU-like parallel computation simulation
                dummy_output_[i] = std::sin(dummy_output_[i]) * std::cos(dummy_output_[i]);
            }
        } else {
            // Simulate CPU computation
            for (size_t i = 0; i < dummy_output_.size(); ++i) {
                dummy_output_[i] = sum * static_cast<float>(i) / 1000.0f;
                // Add some CPU computation
                dummy_output_[i] = dummy_output_[i] * dummy_output_[i];
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "Inference completed successfully!" << std::endl;
        std::cout << "Inference time: " << duration.count() << " microseconds" << std::endl;

        // Print some output statistics for debugging
        float max_output = *std::max_element(dummy_output_.begin(), dummy_output_.end());
        float min_output = *std::min_element(dummy_output_.begin(), dummy_output_.end());
        std::cout << "Output range: [" << min_output << ", " << max_output << "]" << std::endl;

        return true;
    }

    void PrintModelInfo() {
        std::cout << "\n=== Model Information ===" << std::endl;
        std::cout << "Model path: " << model_path_ << std::endl;
        std::cout << "Input size: " << dummy_input_.size() << " elements" << std::endl;
        std::cout << "Output size: " << dummy_output_.size() << " elements" << std::endl;
        std::cout << "Acceleration: " << (use_gpu_ ? "GPU (Metal)" : "CPU") << std::endl;
        std::cout << "==========================" << std::endl;
    }

    // Debug helper functions
    const std::vector<float>& GetInput() const { return dummy_input_; }
    const std::vector<float>& GetOutput() const { return dummy_output_; }
    bool IsUsingGpu() const { return use_gpu_; }
};

void RunBenchmark(const std::string& model_path, int num_iterations = 10) {
    std::cout << "\n=== Running CPU/GPU Benchmark ===" << std::endl;

    // Test CPU version
    {
        std::cout << "\n--- CPU Benchmark ---" << std::endl;
        SimpleInference cpu_inference(model_path, false);

        if (!cpu_inference.LoadModel()) {
            std::cerr << "Failed to load model for CPU benchmark" << std::endl;
            return;
        }

        cpu_inference.PrintModelInfo();

        std::vector<double> cpu_times;
        for (int i = 0; i < num_iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            bool success = cpu_inference.RunInference();
            auto end = std::chrono::high_resolution_clock::now();

            if (success) {
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                cpu_times.push_back(duration.count());
            }
        }

        if (!cpu_times.empty()) {
            double cpu_avg = 0.0;
            for (double time : cpu_times) cpu_avg += time;
            cpu_avg /= cpu_times.size();

            std::cout << "CPU Average inference time: " << cpu_avg << " microseconds" << std::endl;
        }
    }

    // Test GPU version
    {
        std::cout << "\n--- GPU (Simulated) Benchmark ---" << std::endl;
        SimpleInference gpu_inference(model_path, true);

        if (!gpu_inference.LoadModel()) {
            std::cerr << "Failed to load model for GPU benchmark" << std::endl;
            return;
        }

        gpu_inference.PrintModelInfo();

        std::vector<double> gpu_times;
        for (int i = 0; i < num_iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            bool success = gpu_inference.RunInference();
            auto end = std::chrono::high_resolution_clock::now();

            if (success) {
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                gpu_times.push_back(duration.count());
            }
        }

        if (!gpu_times.empty()) {
            double gpu_avg = 0.0;
            for (double time : gpu_times) gpu_avg += time;
            gpu_avg /= gpu_times.size();

            std::cout << "GPU (Simulated) Average inference time: " << gpu_avg << " microseconds" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Simple C++ Debugging Example for ARM macOS ===" << std::endl;
    std::cout << "This example demonstrates VSCode debugging setup without complex dependencies" << std::endl;

    if (argc < 2) {
        std::cerr << "\nUsage: " << argv[0] << " <model_path> [benchmark_iterations]" << std::endl;
        std::cerr << "Example: " << argv[0] << " test_model.tflite 10" << std::endl;
        std::cerr << "\nTo get a test model, run:" << std::endl;
        std::cerr << "curl -L -o test_model.tflite 'https://tfhub.dev/google/lite-model/imagenet/mobilenet_v2_100_224/classification/2/metadata/1?lite-format=tflite'" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    int iterations = (argc > 2) ? std::atoi(argv[2]) : 10;

    std::cout << "\nModel path: " << model_path << std::endl;
    std::cout << "Benchmark iterations: " << iterations << std::endl;

    try {
        // Run benchmark
        RunBenchmark(model_path, iterations);

        std::cout << "\n=== Debugging Demo Complete ===" << std::endl;
        std::cout << "Application finished successfully!" << std::endl;
        std::cout << "\n🔍 DEBUGGING NOTES:" << std::endl;
        std::cout << "- Set breakpoints in LoadModel() to debug model loading (line ~45)" << std::endl;
        std::cout << "- Set breakpoints in RunInference() to debug computation (line ~67)" << std::endl;
        std::cout << "- Use VSCode debugger with 'Debug Simple CPU' or 'Debug Simple GPU' configurations" << std::endl;
        std::cout << "- Inspect variables: dummy_input_, dummy_output_, use_gpu_, model_path_" << std::endl;
        std::cout << "- Step through the computation loops to see how values change" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}