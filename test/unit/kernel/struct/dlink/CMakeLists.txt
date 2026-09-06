# dlink test module.
#
# Self-describing: names its test source, the OSZ .c file under test,
# and the include directories this module's TU chain needs:
#   - double_link/include for dlink.h
#   - comm/include         for offset.h (transitively via dlink.h)
# OSZ_STRUCT_DIR / OSZ_KERNEL_DIR are defined in test/CMakeLists.txt
# and inherited here.

set(OSZ_MODULE_INCLUDE_DIRS
    "${OSZ_STRUCT_DIR}/double_link/include"
    "${OSZ_KERNEL_DIR}/comm/include"
)

add_osz_test(test_dlink
    test_dlink.cpp
    "${OSZ_STRUCT_DIR}/double_link/src/dlink.c"
)