# d mem 测试模块（best-fit 分配 + 空闲块合并）
#
# 覆盖两份 OSZ 源码：
#   - kernel/base/mem/bestfit/src/bestfit.c  （被测模块本体）
#   - kernel/struct/double_link/src/dlink.c  （空闲链依赖）
#
# 同一份用例注册两个可执行：
#   test_mem        OSZ_CFG_MEM_DEBUG 未定义（=0，量产配置）
#   test_mem_debug  OSZ_CFG_MEM_DEBUG=1（Debug 配置，追加自检）
# 两者必须给出完全一致的外部行为，仅 Debug 多出内部一致性断言路径。

set(OSZ_MODULE_INCLUDE_DIRS
    "${OSZ_KERNEL_DIR}/base/mem/bestfit/include"
    "${OSZ_STRUCT_DIR}/double_link/include"
    "${OSZ_KERNEL_DIR}/comm/include"
)

set(MEM_OSZ_SRCS
    "${OSZ_KERNEL_DIR}/base/mem/bestfit/src/bestfit.c"
    "${OSZ_STRUCT_DIR}/double_link/src/dlink.c"
)

add_osz_test(test_mem
    test_mem.cpp
    ${MEM_OSZ_SRCS}
)

add_osz_test(test_mem_debug
    test_mem.cpp
    ${MEM_OSZ_SRCS}
)

target_compile_definitions(osz_host_mem_debug PRIVATE OSZ_CFG_MEM_DEBUG=1)
target_compile_definitions(test_mem_debug PRIVATE OSZ_CFG_MEM_DEBUG=1)
