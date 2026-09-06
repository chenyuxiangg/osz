# sortlink test module.
#
# Self-describing: names its test source, every OSZ .c file needed to
# link, and the include directories this module's TU chain needs:
#   - sortlink/include    for sortlink.h, inner_sortlink_err.h
#   - double_link/include for dlink.h (sortlink.h includes dlink.h)
#   - comm/include        for offset.h / align.h / qualifer.h / comm.h
#                         (sortlink.h includes comm.h, dlink.h includes
#                         offset.h)
#
# sortlink.c calls dlink_init / dlink_insert_* / dlink_del_node, so
# dlink.c must be listed as an extra OSZ source here too. (The two .c
# files end up in the same per-module static lib; harmless duplication
# of one .o is much simpler than tracking module-to-module deps.)
# OSZ_STRUCT_DIR / OSZ_KERNEL_DIR are defined in test/CMakeLists.txt
# and inherited here.

set(OSZ_MODULE_INCLUDE_DIRS
    "${OSZ_STRUCT_DIR}/sortlink/include"
    "${OSZ_STRUCT_DIR}/double_link/include"
    "${OSZ_KERNEL_DIR}/comm/include"
)

add_osz_test(test_sortlink
    test_sortlink.cpp
    "${OSZ_STRUCT_DIR}/sortlink/src/sortlink.c"
    "${OSZ_STRUCT_DIR}/double_link/src/dlink.c"
)