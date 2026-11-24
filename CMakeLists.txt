# Minimal CMake configuration for Google Test and Google Mock
# This file provides a minimal setup for building tests with gtest and gmock

cmake_minimum_required(VERSION 3.5)
# 编译工具链的配置必须放置在 project 前
include(${CMAKE_SOURCE_DIR}/construct/cmake/comm.cmake)

project(OSZ LANGUAGES C ASM)

# OSZ 内核只有 C/汇编翻译单元。C++ 语言是否启用取决于工具链是否带 cc1plus：
#   - 精简 RISC-V 发行包常缺 C++ 前端，此时自动跳过 CXX，构建不受影响；
#   - CI 或需要强制完整工具链时，用 -DOSZ_REQUIRE_CXX=TRUE 让配置阶段直接失败。
if(OSZ_REQUIRE_CXX AND NOT OSZ_CROSS_HAS_CXX)
    message(FATAL_ERROR "OSZ_REQUIRE_CXX=TRUE，但工具链缺少 C++ 前端（cc1plus）")
endif()
if(OSZ_CROSS_HAS_CXX)
    enable_language(CXX)
endif()

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 板上系统测试镜像（P0）：默认 OFF，产品构建不受任何影响。
# 打开后全局注入 -DOSZ_CFG_TEST=1，用于 exception_handler 的测试分支与
# OSZ_TEST 段的 KEEP 校验；单独用 build-test 目录构建，避免污染产品构建目录。
option(OSZ_TEST_BUILD "Build on-target system test image (target_qemu_test)" OFF)
if(OSZ_TEST_BUILD)
    add_compile_definitions(OSZ_CFG_TEST=1)
    message(STATUS "OSZ_TEST_BUILD=ON: building on-target system test image")
endif()

add_subdirectory(arch)
add_subdirectory(kernel)

if(OSZ_TEST_BUILD)
    add_subdirectory(test/sys)
endif()

add_subdirectory(targets)