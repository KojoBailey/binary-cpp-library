cmake_minimum_required(VERSION 3.19)

project(binary VERSION 0.1.0 DESCRIPTION "C++ Binary lib")

add_library(binary INTERFACE)

target_include_directories(binary
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(binary PROPERTIES CXX_STANDARD 23)

add_library(kojo::binary ALIAS binary)

install(TARGETS binary
        EXPORT binaryTargets
        INCLUDES DESTINATION include)

install(EXPORT binaryTargets
        FILE binaryTargets.cmake
        NAMESPACE kojo::
        DESTINATION lib/cmake/binary)