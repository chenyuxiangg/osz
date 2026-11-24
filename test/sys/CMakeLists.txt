# 板上系统测试（on-target system test）
#
# 本目录只做一件事：把 test/sys/cases/ 下的用例源文件收集成一个 CMake 变量，
# 供 targets/<board>/test 的测试镜像使用。harness 不用库的形式链接，而是把
# 源码直接编进可执行文件 —— 因为用例靠 .osz_test 段的静态注册被发现，一旦
# 走静态库/--gc-sections，没被引用的目标文件会被整段丢弃。
#
# 由根 CMakeLists 在 OSZ_TEST_BUILD=ON 时 add_subdirectory 进来。

file(GLOB_RECURSE OSZ_TEST_CASE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/cases/*.c
)

list(LENGTH OSZ_TEST_CASE_SOURCES OSZ_TEST_CASE_FILE_COUNT)
message(STATUS "on-target test: discovered ${OSZ_TEST_CASE_FILE_COUNT} case file(s)")

set(OSZ_TEST_CASE_SOURCES  "${OSZ_TEST_CASE_SOURCES}"  CACHE INTERNAL "on-target test case sources"  FORCE)
set(OSZ_TEST_HARNESS_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/harness"  CACHE INTERNAL "on-target test harness dir" FORCE)
