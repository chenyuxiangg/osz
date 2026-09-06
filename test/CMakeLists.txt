# OSZ unit-test build (host-side, GoogleTest)
#
# This is a STANDALONE CMake project: it uses the HOST compiler (gcc/g++),
# not the RISC-V cross toolchain that builds the actual kernel. It compiles
# selected OSZ source files natively and links them with GoogleTest.
#
# Configuration / build / run:
#     cmake -S test -B build/test
#     cmake --build build/test -j
#     ctest --test-dir build/test --output-on-failure
#
# Or use the wrapper:    ./test/runtest.sh
#
# Selecting which modules to BUILD (vs. which to RUN):
#     cmake -S test -B build/test -DOSZ_TEST_MODULES="dlink;strtoul"
#     cmake -S test -B build/test -DOSZ_TEST_MODULES="none"   # configure only
#     cmake -S test -B build/test                            # builds all
#
# Run-time selection (without re-configuring) goes through the wrapper
# script: ./test/runtest.sh --module <name> --gtest-filter <pattern>.
#
# Per-module layout (mirrors kernel/):
#   test/unit/kernel/<category>/<module>/CMakeLists.txt   registers test_<module>
#   test/unit/kernel/<category>/<module>/test_<module>.cpp
# The leaf directory name `<module>` is the ctest label. <category>
# (e.g. comm / struct) is purely cosmetic — it mirrors the source tree
# for visual parity and does NOT affect module discovery or labels.
#
# Each module's CMakeLists.txt is fully self-describing: it names both
# its test source(s) and the OSZ .c file(s) under test. Selection
# (OSZ_TEST_MODULES) controls which modules get built end-to-end, so
# there is NO top-level "list of OSZ sources" to keep in sync.
#
# Notes:
# - test/include/ provides host-side stubs for OSZ headers (assert.h,
#   menuconfig.h, printf.h, ...). The stubs shadow the OSZ kernel
#   headers via the include-path scheme described below.
# - test/stubs/assert_stub.c provides the bare `void assert()` symbol
#   for OSZ code paths that bypass the `ASSERT` macro.

cmake_minimum_required(VERSION 3.14)
project(OSZ_TEST LANGUAGES C CXX)

# ---- Module selection -----------------------------------------------------
# OSZ_TEST_MODULES controls which test executables are BUILT. Allowed:
#   `all` (default)    - every test/unit/<name>/CMakeLists.txt is loaded.
#   `none`             - no test modules are built (project still configures
#                        and compiles gtest + osz_host libs).
#   `mod1;mod2;...`    - semicolon-separated whitelist of directory names
#                        under test/unit/.
# This is independent of run-time selection, which is handled by
# runtest.sh --module / ctest -L / --gtest-filter.
set(OSZ_TEST_MODULES "all" CACHE STRING
    "Test modules to build: 'all', 'none', or ';' -separated names under test/unit/.")

# ---- Compiler settings ----------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Useful warnings when developing tests
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# CMake's Unix Makefiles / Ninja generators consolidate consecutive
# `-iquote` flags into a single one with multiple paths
# (`-iquote dir1 dir2 ...`). GCC's `-iquote` only honours the FIRST path
# in such a consolidated flag, so every directory after the first stops
# being searchable for `#include "..."`. To work around this we install
# a compiler-launcher wrapper that expands the consolidated `-iquote`
# back into one flag per directory before invoking the real compiler.
set(QUOTE_WRAPPER "${CMAKE_CURRENT_SOURCE_DIR}/cmake/quote_wrapper.sh")
if(NOT EXISTS "${QUOTE_WRAPPER}")
    message(FATAL_ERROR "quote_wrapper.sh not found at ${QUOTE_WRAPPER}")
endif()
set(CMAKE_C_COMPILER_LAUNCHER   "${QUOTE_WRAPPER}" CACHE FILEPATH "C compiler launcher (expands -iquote)")
set(CMAKE_CXX_COMPILER_LAUNCHER "${QUOTE_WRAPPER}" CACHE FILEPATH "C++ compiler launcher (expands -iquote)")

# ---- Paths to OSZ sources -------------------------------------------------
set(OSZ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(OSZ_KERNEL_DIR "${OSZ_ROOT}/kernel")
set(OSZ_STRUCT_DIR "${OSZ_KERNEL_DIR}/struct")

# All OSZ-side header directories are added via `-iquote` (not `-I`):
# `-iquote` paths are searched ONLY for `#include "..."`, which is
# what keeps glibc's `<stdint.h>` / `<stdarg.h>` / `<assert.h>` reachable
# via the angle-bracket form, while still letting OSZ source find its
# bare-metal counterparts via the quoted form. If we used `-I`, then
# `<stdint.h>` would find our stub first, the stub's `#include <stdint.h>`
# re-include would be blocked by its own guard, and glibc's stdint.h
# would never be reached — leaving `uint64_t` etc. undefined.
#
# `OSZ_HOST_COMMON_INCLUDE_DIRS` holds ONLY truly universal headers —
# the host stubs every translation unit needs. Module-specific headers
# (e.g. `double_link/include`, `sortlink/include`) are declared by each
# module's CMakeLists.txt via `OSZ_MODULE_INCLUDE_DIRS` (see
# test/unit/CMakeLists.txt). This keeps the OSZ-side include list
# 100% local to where the dependency actually exists.

set(OSZ_HOST_COMMON_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"            # host stubs (FIRST)
)

# ---- Helper: register an OSZ host library target -------------------------
# Adds the preinclude that pulls glibc's angle-bracket headers before
# any OSZ stub gets a chance to shadow them. `-iquote` flags are NOT
# added here — see `_apply_osz_includes` in test/unit/CMakeLists.txt,
# which adds ALL iquote paths (common + per-module) in a SINGLE call
# to avoid CMake's adjacent-flag consolidation breaking the order.
#
# This function may be called from a deeply-nested add_subdirectory()
# (e.g. test/unit/kernel/struct/dlink/), so the preinclude path uses
# CMAKE_SOURCE_DIR (the topmost source dir, always test/) rather than
# CMAKE_CURRENT_SOURCE_DIR (which would track the per-module dir).
function(add_osz_host_lib name)
    add_library(${name} STATIC ${ARGN})

    target_compile_options(${name} PRIVATE
        "-include" "${CMAKE_SOURCE_DIR}/include/osz_host_preinclude.h"
    )
    target_compile_definitions(${name} PUBLIC OSZ_TESTING=1)
endfunction()

# ---- Host-side stubs (link only, no headers exposed) ---------------------
# `assert_stub.c` provides the bare `void assert()` symbol so any OSZ code
# path that references the bare function (instead of going through the
# `ASSERT` macro) still links on the host. Built unconditionally because
# every test binary needs it.
add_osz_host_lib(osz_host_stubs
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs/assert_stub.c"
)
# Stubs also need the OSZ host-stubs include dir so the preinclude's
# `#include "assert.h"` etc. finds the test/include/ stubs. Since this
# top-level target has no module-specific includes, just the common ones
# are applied.
set(_stub_iquote_opts)
foreach(dir ${OSZ_HOST_COMMON_INCLUDE_DIRS})
    list(APPEND _stub_iquote_opts "-iquote" "${dir}")
endforeach()
target_compile_options(osz_host_stubs PRIVATE ${_stub_iquote_opts})

# OSZ source files are NOT listed here. Each test module owns its own
# sources and tells `add_osz_test` (in unit/CMakeLists.txt) which OSZ .c
# file(s) to compile alongside its test source. The selection (OSZ_TEST_MODULES)
# therefore drives which OSZ source files actually get built — there is
# no global "list of all modules" to keep in sync.

# ---- GoogleTest via FetchContent -----------------------------------------
include(FetchContent)

# Suppress GoogleTest's own install rules — we only want the build target.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK    ON  CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(unit)
