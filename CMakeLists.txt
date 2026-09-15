cmake_minimum_required(VERSION 3.20)

project(LockFreeRingBuffer
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library target
add_library(LockFreeRingBuffer INTERFACE)
add_library(LockFreeRingBuffer::LockFreeRingBuffer ALIAS LockFreeRingBuffer)

target_include_directories(LockFreeRingBuffer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_sources(LockFreeRingBuffer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lfrb.hpp>
)

# Testing configuration
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
