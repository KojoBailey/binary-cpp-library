cmake_minimum_required(VERSION 3.19)

project(binary-cpp-library VERSION 0.1.0 DESCRIPTION "C++ Binary lib")

# Create shared library
add_library(binary-cpp-library INTERFACE)

# Include header files
target_include_directories(binary-cpp-library INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(binary-cpp-library PROPERTIES CXX_STANDARD 23)