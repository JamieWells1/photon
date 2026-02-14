# Standalone Tests CMakeLists.txt
# Run from this directory: cmake -B build && cmake --build build && ./build/test_example

cmake_minimum_required(VERSION 3.13)
project(photon_tests C)

set(CMAKE_C_STANDARD 11)

# Fetch Unity testing framework
include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
)

FetchContent_MakeAvailable(unity)

# Create test executable
add_executable(test_example
    test_example.c
)

# Link Unity
target_link_libraries(test_example
    unity
)

# Add to CTest
enable_testing()
add_test(NAME ExampleTests COMMAND test_example)
