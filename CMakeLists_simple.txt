cmake_minimum_required(VERSION 3.20)
project(simple_debug_example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Debug for better debugging experience
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ARM macOS specific settings
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    message(STATUS "Configuring for ARM macOS (Apple Silicon)")
    set(CMAKE_OSX_ARCHITECTURES arm64)

    # Enable debug symbols and optimization settings for debugging
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

    # Metal framework for GPU acceleration (even though we're just simulating)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)

    message(STATUS "Found Metal framework: ${METAL_FRAMEWORK}")
endif()

# Create a simple executable for debugging demonstration
add_executable(simple_debug_example main_simple.cpp)

# Include directories (minimal - just standard library)
target_include_directories(simple_debug_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with frameworks (to demonstrate framework linking)
if(APPLE)
    target_link_libraries(simple_debug_example
        PRIVATE
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        "-framework Accelerate"
    )
endif()

# Compiler definitions for debugging
target_compile_definitions(simple_debug_example PRIVATE
    SIMPLE_DEBUG_EXAMPLE=1
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<PLATFORM_ID:Darwin>:MACOS_BUILD=1>
)

# Set target properties for optimal debugging
set_target_properties(simple_debug_example PROPERTIES
    # Generate debug symbols
    DEBUG_POSTFIX "_debug"

    # Enable runtime type information for better debugging
    CXX_RTTI ON

    # Position independent code
    POSITION_INDEPENDENT_CODE ON

    # Make debugging easier
    COMPILE_WARNING_AS_ERROR OFF
)

# Create a version that demonstrates Metal framework usage
add_executable(simple_debug_example_metal main_simple.cpp)

target_include_directories(simple_debug_example_metal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    target_link_libraries(simple_debug_example_metal
        PRIVATE
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        "-framework Accelerate"
        "-framework CoreGraphics"
    )
endif()

target_compile_definitions(simple_debug_example_metal PRIVATE
    SIMPLE_DEBUG_EXAMPLE=1
    USE_METAL_SIMULATION=1
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<PLATFORM_ID:Darwin>:MACOS_BUILD=1>
)

set_target_properties(simple_debug_example_metal PROPERTIES
    DEBUG_POSTFIX "_metal_debug"
    CXX_RTTI ON
    POSITION_INDEPENDENT_CODE ON
)

# Print configuration summary
message(STATUS "=== Simple Debug Build Configuration ===")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "Targets: simple_debug_example, simple_debug_example_metal")
message(STATUS "========================================")