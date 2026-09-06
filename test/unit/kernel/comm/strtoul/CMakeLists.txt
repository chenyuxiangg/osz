# strtoul test module.
#
# Self-describing: names its test source, the OSZ .c file under test,
# and the include directories this module's TU chain needs (here just
# `comm/include` for `strtoul.h`). OSZ_KERNEL_DIR is defined in
# test/CMakeLists.txt and inherited here.

set(OSZ_MODULE_INCLUDE_DIRS
    "${OSZ_KERNEL_DIR}/comm/include"
)

add_osz_test(test_strtoul
    test_strtoul.cpp
    "${OSZ_KERNEL_DIR}/comm/src/strtoul.c"
)