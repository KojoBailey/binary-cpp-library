# metadata & version info
cmake_minimum_required(VERSION 3.19)
project(binary VERSION 0.1.0 DESCRIPTION "Library for easily interacting with binary data in C++.")
set(CMAKE_CXX_STANDARD 23)

option(ENABLE_SANITIZERS "Enable ASan + UBSan" ON)

# .cpp source files
set(SOURCES
	src/binary.cpp
	src/binary_view.cpp
)
add_library(binary STATIC ${SOURCES})

# .hpp header files
target_include_directories(binary
	PUBLIC
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

set(WARNING_FLAGS -Wall -Wextra -Wpedantic)
target_compile_options(binary PRIVATE ${WARNING_FLAGS})

# sandbox for manual testing
option(BUILD_SANDBOX "Build the sandbox executable." ON)
if (BUILD_SANDBOX)
	add_executable(binary_lib_test test/main.cpp)
	target_link_libraries(binary_lib_test PRIVATE binary)
	target_compile_options(binary_lib_test PRIVATE ${WARNING_FLAGS})

	if (ENABLE_SANITIZERS)
		set(SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
		target_compile_options(binary_lib_test PRIVATE ${SANITIZER_FLAGS})
		target_link_options(binary_lib_test PRIVATE ${SANITIZER_FLAGS})
	endif()
endif()
