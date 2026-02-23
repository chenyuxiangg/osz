# riscv32-toolchain.cmake
cmake_minimum_required(VERSION 3.15)
# 设置系统信息
set(CMAKE_SYSTEM_NAME Generic)  # 不使用操作系统
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# 工具链前缀（根据你的实际工具链调整）
set(TOOLCHAIN_PATH "${CMAKE_SOURCE_DIR}/tools/toolchain/riscv32-unknown-elf")
set(TOOLCHAIN_PREFIX "riscv32-unknown-elf")

# 编译器
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++" CACHE FILEPATH "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc" CACHE FILEPATH "Assembler" FORCE)

# 设置工具链根目录
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH})
set(CMAKE_SYSROOT ${TOOLCHAIN_PATH}/riscv32-unknown-elf)

# 静态库工具
set(CMAKE_AR "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-ranlib")
# 二进制操作工具
set(CMAKE_STRIP "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-strip")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-objdump")
set(CMAKE_NM "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-nm")
set(CMAKE_READELF "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-readelf")

# RISC-V 特定选项
set(RISCV_ARCH "rv32g")
set(RISCV_ABI "ilp32d")

# 设置编译标志
set(CMAKE_C_FLAGS "-march=${RISCV_ARCH} -mabi=${RISCV_ABI} -ffreestanding -nostdlib -g" CACHE STRING "C flags")
set(CMAKE_CXX_FLAGS "-march=${RISCV_ARCH} -mabi=${RISCV_ABI} -ffreestanding -nostdlib -fno-exceptions -fno-rtti -g" CACHE STRING "C++ flags")
set(CMAKE_ASM_FLAGS "-march=${RISCV_ARCH} -mabi=${RISCV_ABI}" CACHE STRING "Assembler flags")

# 设置链接标志
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -nostartfiles -Wl,--gc-sections -T${CMAKE_SOURCE_DIR}/build/osz.ld" CACHE STRING "Linker flags")

# 设置库路径（重要！）
set(CMAKE_C_LINK_FLAGS "-L${TOOLCHAIN_PATH}/lib/gcc/riscv32-unknown-elf/14.2.0 -L${TOOLCHAIN_PATH}/riscv32-unknown-elf/lib")

# 跳过编译器测试（关键！）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 搜索模式
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
