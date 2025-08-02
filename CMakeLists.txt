cmake_minimum_required(VERSION 3.19)
project(binary VERSION 0.1.0 DESCRIPTION "C++ Binary lib")
set(CMAKE_CXX_STANDARD 20)

add_library(binary INTERFACE
        include/kojo/binary.hpp
)

target_include_directories(binary
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_library(kojo::binary ALIAS binary)

install(TARGETS binary
        EXPORT binaryTargets
        INCLUDES DESTINATION include)

install(EXPORT binaryTargets
        FILE binaryTargets.cmake
        NAMESPACE kojo::
        DESTINATION lib/cmake/binary)