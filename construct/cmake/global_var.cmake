# =================================== Global Var ===============================
include(${CMAKE_SOURCE_DIR}/construct/cmake/cfg.cmake)

set(OSZ_KERNEL_DIR ${CMAKE_SOURCE_DIR}/kernel)
set(OSZ_KERNEL_BASE_DIR ${OSZ_KERNEL_DIR}/base)
set(OSZ_KERNEL_COMM_DIR ${OSZ_KERNEL_DIR}/comm)
set(OSZ_KERNEL_DEBUG_DIR ${OSZ_KERNEL_DIR}/debug)
set(OSZ_KERNEL_DFX_DIR ${OSZ_KERNEL_DIR}/dfx)
set(OSZ_KERNEL_DRIVERS_DIR ${OSZ_KERNEL_DIR}/drivers)
set(OSZ_KERNEL_SHELL_DIR ${OSZ_KERNEL_DIR}/shell)
set(OSZ_KERNEL_STARTUP_DIR ${OSZ_KERNEL_DIR}/startup)
set(OSZ_KERNEL_STRUCT_DIR ${OSZ_KERNEL_DIR}/struct)
set(OSZ_PLATFORM_DIR ${CMAKE_SOURCE_DIR}/platform)
set(OSZ_TARGETS_DIR ${CMAKE_SOURCE_DIR}/targets)
set(OSZ_BUILD_DIR ${CMAKE_SOURCE_DIR}/construct)
set(OSZ_TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)

set(COMPILER_GLOBAL_INCLUDE 
    ${OSZ_PLATFORM_DIR}/qemu_virt/include
    ${OSZ_TARGETS_DIR}/platform/include
    ${OSZ_KERNEL_COMM_DIR}/include
    ${OSZ_KERNEL_DEBUG_DIR}/include
)
# ==============================================================================