# riscv32_toolchain.cmake
# 交叉工具链自适应入口：仓库内不再写死前缀、目录结构与 gcc 版本。
#
# 工具链选择优先级（高 → 低）：
#   1. cmake -D 参数
#   2. 同名环境变量（OSZ_CROSS_*）
#   3. 仓库内 tools/toolchain（可选，通常是符号链接或解压目录，不纳入 git）
#   4. 系统 PATH
#
# 典型用法（换一台机器只需设置两个环境变量，不改任何受版本控制的行）：
#   export OSZ_CROSS_ROOT=/opt/xpack-riscv-none-elf-gcc-15.2.0-1
#   export OSZ_CROSS_PREFIX=riscv-none-elf
#   ./construct/build_cmake.sh target_qemu
# 或一次性命令行：
#   cmake -DOSZ_CROSS_ROOT=... -DOSZ_CROSS_PREFIX=... -B build

cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# ---------------------------------------------------------------- 可配置项
set(OSZ_CROSS_PREFIX "" CACHE STRING "工具链前缀，如 riscv32-unknown-elf / riscv-none-elf；留空则自动探测")
set(OSZ_CROSS_ROOT   "" CACHE PATH   "工具链根目录（含 bin/）；留空则依次查 tools/toolchain 与 PATH")
set(OSZ_CROSS_ARCH   "rv32g" CACHE STRING "编译目标架构")
set(OSZ_CROSS_ABI    "ilp32d" CACHE STRING "编译目标 ABI")
set(OSZ_REQUIRE_CXX  OFF CACHE BOOL "TRUE = 工具链必须带 C++ 前端（cc1plus），缺则配置失败")

# 环境变量只在未通过 -D 指定时生效，保证 cmake -D 的优先级最高
if(NOT OSZ_CROSS_PREFIX AND DEFINED ENV{OSZ_CROSS_PREFIX})
    set(OSZ_CROSS_PREFIX "$ENV{OSZ_CROSS_PREFIX}" CACHE STRING "" FORCE)
endif()
if(NOT OSZ_CROSS_ROOT AND DEFINED ENV{OSZ_CROSS_ROOT})
    set(OSZ_CROSS_ROOT "$ENV{OSZ_CROSS_ROOT}" CACHE PATH "" FORCE)
endif()
if(DEFINED ENV{OSZ_CROSS_ARCH} AND OSZ_CROSS_ARCH STREQUAL "rv32g")
    set(OSZ_CROSS_ARCH "$ENV{OSZ_CROSS_ARCH}" CACHE STRING "" FORCE)
endif()
if(DEFINED ENV{OSZ_CROSS_ABI} AND OSZ_CROSS_ABI STREQUAL "ilp32d")
    set(OSZ_CROSS_ABI "$ENV{OSZ_CROSS_ABI}" CACHE STRING "" FORCE)
endif()
if(DEFINED ENV{OSZ_REQUIRE_CXX})
    set(OSZ_REQUIRE_CXX "$ENV{OSZ_REQUIRE_CXX}" CACHE BOOL "" FORCE)
endif()

set(RISCV_ARCH "${OSZ_CROSS_ARCH}")
set(RISCV_ABI  "${OSZ_CROSS_ABI}")

# ---------------------------------------------------------------- 前缀探测
# 用户给定前缀优先；否则按常见发行包的命名逐个尝试（xPack 用 riscv-none-elf，
# 官方 riscv-gnu-toolchain 用 riscv32-unknown-elf / riscv64-unknown-elf）。
set(_osz_prefix_candidates "")
if(OSZ_CROSS_PREFIX)
    list(APPEND _osz_prefix_candidates "${OSZ_CROSS_PREFIX}")
endif()
list(APPEND _osz_prefix_candidates riscv32-unknown-elf riscv-none-elf riscv64-unknown-elf)

set(TOOLCHAIN_PREFIX "")
foreach(_p ${_osz_prefix_candidates})
    # 搜索顺序：用户根目录 → 仓库 tools/toolchain（扁平 / 带前缀两种布局）→ PATH
    set(_osz_hints "")
    if(OSZ_CROSS_ROOT)
        list(APPEND _osz_hints "${OSZ_CROSS_ROOT}/bin")
    endif()
    list(APPEND _osz_hints
        "${CMAKE_SOURCE_DIR}/tools/toolchain/bin"
        "${CMAKE_SOURCE_DIR}/tools/toolchain/${_p}/bin")

    find_program(_osz_gcc NAMES "${_p}-gcc" PATHS ${_osz_hints} NO_DEFAULT_PATH)
    if(NOT _osz_gcc)
        find_program(_osz_gcc NAMES "${_p}-gcc")     # 兜底：PATH
    endif()
    if(_osz_gcc)
        set(TOOLCHAIN_PREFIX "${_p}")
        get_filename_component(OSZ_TOOLCHAIN_BIN "${_osz_gcc}" DIRECTORY)
        break()
    endif()
endforeach()

if(NOT TOOLCHAIN_PREFIX)
    message(FATAL_ERROR
        "未找到 RISC-V 交叉工具链。请设置 OSZ_CROSS_ROOT / OSZ_CROSS_PREFIX，"
        "或把 <prefix>-gcc 加入 PATH。已尝试前缀：${_osz_prefix_candidates}")
endif()
message(STATUS "[TOOLCHAIN] 前缀 ${TOOLCHAIN_PREFIX}，bin 目录 ${OSZ_TOOLCHAIN_BIN}")

# ---------------------------------------------------------------- 工具注册
# 注意：find_program 会缓存结果变量，每个工具必须用独立变量名，
# 否则后续工具会命中第一个工具的缓存值（曾导致 ar 被错认成 gcc）。
foreach(_tool gcc g++ ar ranlib strip objcopy objdump nm readelf size addr2line)
    set(_var "OSZ_TOOL_${_tool}")
    find_program(${_var} NAMES "${TOOLCHAIN_PREFIX}-${_tool}"
                 PATHS "${OSZ_TOOLCHAIN_BIN}" NO_DEFAULT_PATH)
    if(NOT ${_var})
        find_program(${_var} NAMES "${TOOLCHAIN_PREFIX}-${_tool}")
    endif()
    if(${_var})
        if(_tool STREQUAL "gcc")
            set(CMAKE_C_COMPILER   "${${_var}}" CACHE FILEPATH "C compiler" FORCE)
            set(CMAKE_ASM_COMPILER "${${_var}}" CACHE FILEPATH "Assembler" FORCE)
        elseif(_tool STREQUAL "g++")
            set(CMAKE_CXX_COMPILER "${${_var}}" CACHE FILEPATH "C++ compiler" FORCE)
        else()
            string(TOUPPER "${_tool}" _tool_upper)
            set(CMAKE_${_tool_upper} "${${_var}}" CACHE FILEPATH "" FORCE)
        endif()
    endif()
endforeach()

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "找不到 ${TOOLCHAIN_PREFIX}-gcc")
endif()
if(NOT CMAKE_AR OR NOT CMAKE_OBJCOPY)
    message(FATAL_ERROR "工具链不完整：缺少 ${TOOLCHAIN_PREFIX}-ar 或 -objcopy")
endif()

# C++ 前端是否存在（精简发行包常缺 cc1plus）：-print-prog-name 未解析成绝对路径即视为缺失
if(CMAKE_CXX_COMPILER)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-prog-name=cc1plus
                    OUTPUT_VARIABLE _osz_cc1plus OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    if(IS_ABSOLUTE "${_osz_cc1plus}" AND EXISTS "${_osz_cc1plus}")
        set(OSZ_CROSS_HAS_CXX TRUE CACHE INTERNAL "toolchain provides cc1plus" FORCE)
    else()
        set(OSZ_CROSS_HAS_CXX FALSE CACHE INTERNAL "toolchain lacks cc1plus" FORCE)
    endif()
    message(STATUS "[TOOLCHAIN] C++ 前端: ${OSZ_CROSS_HAS_CXX}")
else()
    set(OSZ_CROSS_HAS_CXX FALSE CACHE INTERNAL "no C++ driver" FORCE)
endif()

# ---------------------------------------------------------------- 编译/链接参数
set(CMAKE_C_FLAGS   "-march=${RISCV_ARCH} -mabi=${RISCV_ABI} -ffreestanding -nostdlib -g"
    CACHE STRING "C flags")
set(CMAKE_ASM_FLAGS "-march=${RISCV_ARCH} -mabi=${RISCV_ABI}" CACHE STRING "Assembler flags")
set(CMAKE_CXX_FLAGS "-march=${RISCV_ARCH} -mabi=${RISCV_ABI} -ffreestanding -nostdlib -fno-exceptions -fno-rtti -g"
    CACHE STRING "C++ flags")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -nostartfiles -Wl,--gc-sections -T${CMAKE_SOURCE_DIR}/construct/osz.ld"
    CACHE STRING "Linker flags")

# sysroot 与 libgcc 位置由编译器自己回答，不写死目录名与 gcc 版本号
execute_process(COMMAND ${CMAKE_C_COMPILER} --print-sysroot
                OUTPUT_VARIABLE _osz_sysroot OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(_osz_sysroot AND IS_DIRECTORY "${_osz_sysroot}")
    set(CMAKE_SYSROOT "${_osz_sysroot}")
endif()

execute_process(COMMAND ${CMAKE_C_COMPILER} -march=${RISCV_ARCH} -mabi=${RISCV_ABI}
                        -print-file-name=libgcc.a
                OUTPUT_VARIABLE _osz_libgcc OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
set(CMAKE_C_LINK_FLAGS "")
if(IS_ABSOLUTE "${_osz_libgcc}" AND EXISTS "${_osz_libgcc}")
    get_filename_component(_osz_libdir "${_osz_libgcc}" DIRECTORY)
    set(CMAKE_C_LINK_FLAGS "-L${_osz_libdir}")

    # 把工具链根目录交给 find_root_path，避免把宿主库误当目标库
    get_filename_component(_osz_toolroot "${OSZ_TOOLCHAIN_BIN}" DIRECTORY)
    set(CMAKE_FIND_ROOT_PATH "${_osz_toolroot}")
endif()

# 交叉环境下的 find_* 行为
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 跳过编译器可执行性测试（裸机目标无法运行宿主测试程序）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
