# Minimal CMake configuration for Google Test and Google Mock
# This file provides a minimal setup for building tests with gtest and gmock

cmake_minimum_required(VERSION 3.5)
# 编译工具链的配置必须放置在 project 前
include(${CMAKE_SOURCE_DIR}/build/cmake/comm.cmake)

project(OSZ LANGUAGES C CXX ASM)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(arch)
add_subdirectory(kernel)
add_subdirectory(targets)