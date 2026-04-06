# metadata & version info
cmake_minimum_required(VERSION 3.19)
project(binary VERSION 0.1.0 DESCRIPTION "Library for easily interacting with binary data in C++.")
set(CMAKE_CXX_STANDARD 23) # -std=c++23

# .hpp header file
add_library(binary INTERFACE
	include/kojo/binary.hpp
)
target_include_directories(binary
INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

# namespaced alias for lib users
add_library(kojo::binary ALIAS binary)

# lib system installation
install(TARGETS binary
	EXPORT binaryTargets
	INCLUDES DESTINATION include
)
install(EXPORT binaryTargets
	FILE binaryTargets.cmake
	NAMESPACE kojo::
	DESTINATION lib/cmake/binary
)

# sandbox for manual testing
# option(BUILD_SANDBOX "Build the sandbox executable." ON)
# if (BUILD_SANDBOX)
# 	add_executable(binary_lib_test test/main.cpp)
# 	target_link_libraries(binary_lib_test PRIVATE binary)
# endif()