# =============================================================================
# target_qemu_test —— 板上系统测试镜像（autorun）
#
# 与产品 target_qemu 的唯一差别：
#   * main/app_main 换成 harness 驱动（跑完套件后打哨兵，然后空转）
#   * 额外编进 test/sys/harness + test/sys/cases
#   * 全局多了 -DOSZ_CFG_TEST=1（由根 CMake 在 OSZ_TEST_BUILD=ON 时注入）
#
# 产品镜像 target_qemu 的依赖与源文件一行不动。
# =============================================================================

define_module(
    NAME target_qemu_test
    TYPE EXECUTABLE
    SOURCES
        src/test_main.c
        ${OSZ_TEST_HARNESS_DIR}/src/assert.c
        ${OSZ_TEST_HARNESS_DIR}/src/report.c
        ${OSZ_TEST_HARNESS_DIR}/src/runner.c
        ${OSZ_TEST_HARNESS_DIR}/src/cmd.c
        ${OSZ_TEST_CASE_SOURCES}
    ASM_SOURCES
        ${OSZ_TEST_HARNESS_DIR}/arch/setjmp.S
    DISASSEMBLE_TOOL
        ${CMAKE_OBJDUMP}
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
        ${OSZ_TEST_HARNESS_DIR}/include
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        ${OSZ_KERNEL_MODULES}
        k_dfx
        k_base_swt
        k_drv_timer
    ENABLE_QEMU_RUN OFF
    ENABLE_GDB_DEBUG OFF
)

# =============================================================================
# target_qemu_shelltest —— 产品形态镜像 + 手动 `test` 命令（现场定位用）
#
# 与产品 target_qemu 的差别只在「多编进 harness/cases 并注册一个静态 test 命令」：
#   * 不自动跑套件（autorun 只在 target_qemu_test 里）
#   * 启动后仍是正常的 shell 交互，可 `test list` / `test <suite.name>` 单跑
#   * 产品 target_qemu 的源码与依赖依然一行不动
# =============================================================================
define_module(
    NAME target_qemu_shelltest
    TYPE EXECUTABLE
    SOURCES
        src/shelltest_main.c
        ${OSZ_TEST_HARNESS_DIR}/src/assert.c
        ${OSZ_TEST_HARNESS_DIR}/src/report.c
        ${OSZ_TEST_HARNESS_DIR}/src/runner.c
        ${OSZ_TEST_HARNESS_DIR}/src/cmd.c
        ${OSZ_TEST_CASE_SOURCES}
    ASM_SOURCES
        ${OSZ_TEST_HARNESS_DIR}/arch/setjmp.S
    DISASSEMBLE_TOOL
        ${CMAKE_OBJDUMP}
    PRIVATE_INCLUDES
        ${COMPILER_GLOBAL_INCLUDE}
        ${OSZ_TEST_HARNESS_DIR}/include
    PRIVATE_DEFINITIONS
        ${OSZ_CFG_MACRO}
    PRIVATE_DEPENDENCIES
        ${OSZ_KERNEL_MODULES}
        k_dfx
        k_base_swt
        k_drv_timer
    ENABLE_QEMU_RUN OFF
    ENABLE_GDB_DEBUG OFF
)
