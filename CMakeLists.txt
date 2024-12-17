cmake_minimum_required(VERSION 3.19)

project(binary-cpp-library VERSION 0.1.0 DESCRIPTION "C++ Binary lib")

# Create shared library
add_library(binary-cpp-library INTERFACE)

# Include header files
target_include_directories(binary-cpp-library INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(binary-cpp-library INTERFACE cxx_std_11)