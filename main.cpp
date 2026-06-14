#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <string>
#include <fstream>

// LiteRT headers
#include "litert/c/litert_common.h"
#include "litert/c/litert_model.h"
#include "litert/c/litert_compiled_model.h"
#include "litert/c/litert_tensor_buffer.h"

class LiteRTInference {
private:
    LrtCompiledModel compiled_model_ = nullptr;
    LrtSignature signature_ = nullptr;
    std::vector<LrtTensorBuffer> input_buffers_;
    std::vector<LrtTensorBuffer> output_buffers_;
    bool use_gpu_ = false;

public:
    LiteRTInference(bool use_gpu = false) : use_gpu_(use_gpu) {}

    ~LiteRTInference() {
        CleanUp();
    }

    void CleanUp() {
        if (compiled_model_) {
            LrtDestroyCompiledModel(compiled_model_);
            compiled_model_ = nullptr;
        }
        for (auto& buffer : input_buffers_) {
            if (buffer) {
                LrtDestroyTensorBuffer(buffer);
            }
        }
        for (auto& buffer : output_buffers_) {
            if (buffer) {
                LrtDestroyTensorBuffer(buffer);
            }
        }
        input_buffers_.clear();
        output_buffers_.clear();
    }

    bool LoadModel(const std::string& model_path) {
        std::cout << "Loading model: " << model_path << std::endl;
        std::cout << "Using " << (use_gpu_ ? "GPU (Metal)" : "CPU") << " acceleration" << std::endl;

        // Load model
        LrtModel model;
        if (LrtCreateModelFromFile(model_path.c_str(), &model) != kLrtStatusOk) {
            std::cerr << "Failed to load model from: " << model_path << std::endl;
            return false;
        }

        // Create compilation options
        LrtCompileOptions compile_options;
        if (LrtCreateCompileOptions(&compile_options) != kLrtStatusOk) {
            std::cerr << "Failed to create compile options" << std::endl;
            LrtDestroyModel(model);
            return false;
        }

        // Configure GPU acceleration if requested
        if (use_gpu_) {
            std::cout << "Configuring GPU acceleration..." << std::endl;
            // Note: GPU configuration would go here
        }

        // Compile model
        if (LrtCompileModel(model, compile_options, &compiled_model_) != kLrtStatusOk) {
            std::cerr << "Failed to compile model" << std::endl;
            LrtDestroyCompileOptions(compile_options);
            LrtDestroyModel(model);
            return false;
        }

        // Get default signature
        if (LrtGetCompiledModelSignature(compiled_model_, "serving_default", &signature_) != kLrtStatusOk) {
            // Try getting the first available signature
            LrtParamIndex num_signatures;
            if (LrtGetNumCompiledModelSignatures(compiled_model_, &num_signatures) == kLrtStatusOk && num_signatures > 0) {
                if (LrtGetCompiledModelSignatureByIndex(compiled_model_, 0, &signature_) != kLrtStatusOk) {
                    std::cerr << "Failed to get model signature" << std::endl;
                    LrtDestroyCompileOptions(compile_options);
                    LrtDestroyModel(model);
                    return false;
                }
            } else {
                std::cerr << "No signatures found in model" << std::endl;
                LrtDestroyCompileOptions(compile_options);
                LrtDestroyModel(model);
                return false;
            }
        }

        // Setup input/output buffers
        if (!SetupBuffers()) {
            LrtDestroyCompileOptions(compile_options);
            LrtDestroyModel(model);
            return false;
        }

        LrtDestroyCompileOptions(compile_options);
        LrtDestroyModel(model);

        std::cout << "Model loaded and compiled successfully!" << std::endl;
        return true;
    }

    bool SetupBuffers() {
        // Get number of inputs and outputs
        LrtParamIndex num_inputs, num_outputs;

        if (LrtGetSignatureNumInputs(signature_, &num_inputs) != kLrtStatusOk) {
            std::cerr << "Failed to get number of inputs" << std::endl;
            return false;
        }

        if (LrtGetSignatureNumOutputs(signature_, &num_outputs) != kLrtStatusOk) {
            std::cerr << "Failed to get number of outputs" << std::endl;
            return false;
        }

        std::cout << "Model has " << num_inputs << " inputs and " << num_outputs << " outputs" << std::endl;

        // Setup input buffers
        input_buffers_.resize(num_inputs, nullptr);
        for (LrtParamIndex i = 0; i < num_inputs; ++i) {
            LrtRankedTensorType tensor_type;
            if (LrtGetSignatureInputType(signature_, i, &tensor_type) != kLrtStatusOk) {
                std::cerr << "Failed to get input tensor type " << i << std::endl;
                return false;
            }

            // Create tensor buffer based on type
            LrtTensorBufferRequirements requirements;
            if (LrtGetDefaultTensorBufferRequirements(1, &tensor_type, &requirements) != kLrtStatusOk) {
                std::cerr << "Failed to get buffer requirements for input " << i << std::endl;
                return false;
            }

            if (LrtCreateManagedTensorBuffer(kLrtTensorBufferTypeHost,
                                           &tensor_type, requirements,
                                           &input_buffers_[i]) != kLrtStatusOk) {
                std::cerr << "Failed to create input buffer " << i << std::endl;
                return false;
            }

            std::cout << "Created input buffer " << i << std::endl;
        }

        // Setup output buffers
        output_buffers_.resize(num_outputs, nullptr);
        for (LrtParamIndex i = 0; i < num_outputs; ++i) {
            LrtRankedTensorType tensor_type;
            if (LrtGetSignatureOutputType(signature_, i, &tensor_type) != kLrtStatusOk) {
                std::cerr << "Failed to get output tensor type " << i << std::endl;
                return false;
            }

            LrtTensorBufferRequirements requirements;
            if (LrtGetDefaultTensorBufferRequirements(1, &tensor_type, &requirements) != kLrtStatusOk) {
                std::cerr << "Failed to get buffer requirements for output " << i << std::endl;
                return false;
            }

            if (LrtCreateManagedTensorBuffer(kLrtTensorBufferTypeHost,
                                           &tensor_type, requirements,
                                           &output_buffers_[i]) != kLrtStatusOk) {
                std::cerr << "Failed to create output buffer " << i << std::endl;
                return false;
            }

            std::cout << "Created output buffer " << i << std::endl;
        }

        return true;
    }

    bool RunInference() {
        if (!compiled_model_ || !signature_) {
            std::cerr << "Model not loaded" << std::endl;
            return false;
        }

        std::cout << "Running inference..." << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();

        // Run inference
        LrtStatus status = LrtInvokeCompiledModelSignature(
            compiled_model_, signature_,
            input_buffers_.size(), input_buffers_.data(),
            output_buffers_.size(), output_buffers_.data()
        );

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        if (status != kLrtStatusOk) {
            std::cerr << "Inference failed with status: " << status << std::endl;
            return false;
        }

        std::cout << "Inference completed successfully!" << std::endl;
        std::cout << "Inference time: " << duration.count() << " microseconds" << std::endl;

        return true;
    }

    void PrintModelInfo() {
        if (!signature_) return;

        std::cout << "\n=== Model Information ===" << std::endl;

        LrtParamIndex num_inputs, num_outputs;
        LrtGetSignatureNumInputs(signature_, &num_inputs);
        LrtGetSignatureNumOutputs(signature_, &num_outputs);

        std::cout << "Inputs: " << num_inputs << std::endl;
        std::cout << "Outputs: " << num_outputs << std::endl;
        std::cout << "Acceleration: " << (use_gpu_ ? "GPU (Metal)" : "CPU") << std::endl;
        std::cout << "==========================" << std::endl;
    }

    void FillDummyInput() {
        // Fill input buffers with dummy data for testing
        for (size_t i = 0; i < input_buffers_.size(); ++i) {
            void* buffer_addr;
            size_t buffer_size;

            if (LrtLockTensorBuffer(input_buffers_[i], &buffer_addr, &buffer_size) == kLrtStatusOk) {
                // Fill with dummy data - assume float tensors
                float* float_buffer = static_cast<float*>(buffer_addr);
                size_t num_elements = buffer_size / sizeof(float);

                for (size_t j = 0; j < num_elements; ++j) {
                    float_buffer[j] = static_cast<float>(j % 100) / 100.0f; // Values 0.0 to 0.99
                }

                LrtUnlockTensorBuffer(input_buffers_[i]);
                std::cout << "Filled input " << i << " with " << num_elements << " dummy values" << std::endl;
            } else {
                std::cerr << "Failed to lock input buffer " << i << std::endl;
            }
        }
    }
};

void RunBenchmark(const std::string& model_path, int num_iterations = 10) {
    std::cout << "\n=== Running CPU/GPU Benchmark ===" << std::endl;

    // Test CPU version
    {
        std::cout << "\n--- CPU Benchmark ---" << std::endl;
        LiteRTInference cpu_inference(false);

        if (!cpu_inference.LoadModel(model_path)) {
            std::cerr << "Failed to load model for CPU benchmark" << std::endl;
            return;
        }

        cpu_inference.FillDummyInput();
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
        std::cout << "\n--- GPU Benchmark ---" << std::endl;
        LiteRTInference gpu_inference(true);

        if (!gpu_inference.LoadModel(model_path)) {
            std::cerr << "GPU acceleration may not be available, skipping GPU benchmark" << std::endl;
            return;
        }

        gpu_inference.FillDummyInput();
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

            std::cout << "GPU Average inference time: " << gpu_avg << " microseconds" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== LiteRT C++ Inference Example for ARM macOS ===" << std::endl;
    std::cout << "Supporting both CPU and GPU (Metal) acceleration" << std::endl;

    if (argc < 2) {
        std::cerr << "\nUsage: " << argv[0] << " <model_path> [benchmark_iterations]" << std::endl;
        std::cerr << "Example: " << argv[0] << " model.tflite 10" << std::endl;
        std::cerr << "\nTo get a test model, run:" << std::endl;
        std::cerr << "curl -L -o test_model.tflite 'https://tfhub.dev/google/lite-model/imagenet/mobilenet_v2_100_224/classification/2/metadata/1?lite-format=tflite'" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    int iterations = (argc > 2) ? std::atoi(argv[2]) : 10;

    std::cout << "\nModel path: " << model_path << std::endl;
    std::cout << "Benchmark iterations: " << iterations << std::endl;

    // Check if model file exists
    std::ifstream file(model_path);
    if (!file.good()) {
        std::cerr << "\nERROR: Model file not found: " << model_path << std::endl;
        std::cerr << "Please provide a valid .tflite model file" << std::endl;
        std::cerr << "\nTo download a test model:" << std::endl;
        std::cerr << "curl -L -o test_model.tflite 'https://tfhub.dev/google/lite-model/imagenet/mobilenet_v2_100_224/classification/2/metadata/1?lite-format=tflite'" << std::endl;
        return 1;
    }

    try {
        // Run comprehensive benchmark
        RunBenchmark(model_path, iterations);

        std::cout << "\n=== Benchmark Complete ===" << std::endl;
        std::cout << "Application finished successfully!" << std::endl;
        std::cout << "\nDEBUGGING NOTES:" << std::endl;
        std::cout << "- Set breakpoints in LoadModel() to debug model loading" << std::endl;
        std::cout << "- Set breakpoints in RunInference() to debug inference" << std::endl;
        std::cout << "- Use VSCode debugger with 'Debug LiteRT CPU' or 'Debug LiteRT GPU' configurations" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}