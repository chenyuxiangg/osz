# Intermediate CMakeLists for the OSZ host-side test layout.
#
# This file is loaded by the top-level test/CMakeLists.txt. Its job is
# threefold:
#
#   1. Define the `add_osz_test` helper that all per-module subdirs
#      use to register their test executable AND the OSZ source(s)
#      under test.
#   2. Auto-discover every subdirectory that looks like a test module
#      (contains a CMakeLists.txt and is listed in OSZ_TEST_MODULES).
#   3. `add_subdirectory()` each selected module, in a deterministic
#      order so the ctest output is stable across runs.
#
# `OSZ_TEST_MODULES` is a top-level cache variable (default `all`) set
# in test/CMakeLists.txt. Allowed values:
#   - `all`               every module under unit/ is built.
#   - `none`              no module is built (just configures gtest).
#   - `dlink;sortlink`    semicolon-separated whitelist.
# The whitelist is matched by exact directory name (the module label).
#
# Per-module OSZ sources are NOT listed here — each module declares
# both its test source(s) and the OSZ .c file(s) under test inside its
# own CMakeLists.txt via `add_osz_test`. This keeps the OSZ source
# list 100% local to each module: adding a new module = adding a new
# directory, no edits anywhere else.

# ---- Helper: register an OSZ gtest executable + its OSZ host library ----
# Signature:
#   add_osz_test(<name>
#       <test_source>                 # the gtest .cpp
#       [<osz_source>...]             # one or more OSZ .c files under test
#   )
#
# The caller MAY set `OSZ_MODULE_INCLUDE_DIRS` in its own scope before
# invoking add_osz_test. If set, those directories are added via
# `-iquote` to both the per-module host lib AND the test executable.
# (The always-on host stubs in OSZ_HOST_COMMON_INCLUDE_DIRS are added
# unconditionally.)
#
# Behaviour:
#   - Derives the module label from `<name>` (strip `test_` prefix).
#   - Compiles the OSZ sources into a per-module static library
#     `osz_host_<label>`, so each module gets exactly the .c files it
#     needs — selection therefore drives compilation, not just linking.
#   - Creates the gtest executable from `<test_source>`.
#   - Links the executable to the per-module lib, the always-on
#     `osz_host_stubs`, and GoogleTest.
#   - Applies the same `-iquote` / `-include` preinclude used by the
#     host library (see test/CMakeLists.txt for the rationale).
#   - Registers the test with ctest, tagged with the module label.

# Internal helper: apply the COMMON (host stubs) + per-module
# `-iquote` paths to a target. The caller may set
# `OSZ_MODULE_INCLUDE_DIRS` before calling add_osz_test; this helper
# picks it up.
#
# All `-iquote` flags are added in ONE `target_compile_options` call.
# CMake's Unix Makefiles / Ninja generators consolidate adjacent
# `-iquote` flags into one with multiple paths. Splitting iquote adds
# across multiple calls (e.g. one for stubs + one for module-specific)
# causes CMake to emit the consolidated form interleaved with the
# `-include` preinclude flag, which breaks the `quote_wrapper.sh`
# absorption logic. Keeping everything in one call avoids that.
#
# All paths use `-iquote` (not `-I`) because OSZ headers in
# `comm/include/` define types like `int64_t` that conflict with
# glibc; `-iquote` keeps `<stdint.h>` etc. routed to glibc via system
# paths while still letting OSZ source find its headers via `"..."`.
function(_apply_osz_includes target)
    set(_opts)
    foreach(dir ${OSZ_HOST_COMMON_INCLUDE_DIRS})
        list(APPEND _opts "-iquote" "${dir}")
    endforeach()
    if(DEFINED OSZ_MODULE_INCLUDE_DIRS AND OSZ_MODULE_INCLUDE_DIRS)
        foreach(dir ${OSZ_MODULE_INCLUDE_DIRS})
            list(APPEND _opts "-iquote" "${dir}")
        endforeach()
    endif()
    target_compile_options(${target} PRIVATE ${_opts})
endfunction()

function(add_osz_test name)
    if(ARGC LESS 2)
        message(FATAL_ERROR
            "add_osz_test(${name}): need at least a test source "
            "(usage: add_osz_test(<name> <test.cpp> [<osz.c>...])).")
    endif()

    set(_test_src "${ARGV1}")

    # Remaining args (ARGV2 .. ARGV<ARGC-1>) are OSZ source files.
    # ARGV is the full arg list; strip the first two to get just them.
    set(_osz_srcs ${ARGV})
    list(REMOVE_AT _osz_srcs 0 1)

    string(REGEX REPLACE "^test_" "" _label "${name}")
    if(_label STREQUAL name)
        message(WARNING
            "add_osz_test(${name}): name does not start with 'test_'; "
            "module label would be the full name.")
    endif()
    set(_lib_name "osz_host_${_label}")

    # Per-module static lib. Created only if OSZ sources were given.
    # Both the lib AND the test executable get the per-module includes
    # via _apply_osz_includes, so #include "..." searches for
    # dlink.h / sortlink.h / comm.h etc. work in both.
    if(_osz_srcs)
        add_osz_host_lib(${_lib_name} ${_osz_srcs})
        _apply_osz_includes(${_lib_name})
    endif()

    add_executable(${name} ${_test_src})

    target_link_libraries(${name} PRIVATE
        ${_lib_name}
        osz_host_stubs
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        pthread
    )

    _apply_osz_includes(${name})

    # Same preinclude the host library uses so gtest's `<cstdint>` finds
    # glibc's `int8_t` / `uint64_t` etc. in `::`. The function is called
    # from per-module CMakeLists.txt files inside
    # `unit/kernel/<category>/<name>/`, so CMAKE_CURRENT_SOURCE_DIR is
    # that module dir; the preinclude lives at `test/include/`, four
    # levels up.
    target_compile_options(${name} PRIVATE
        "-include" "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/osz_host_preinclude.h"
    )

    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS "${_label}")
endfunction()

# ---- Auto-discovery & selection -----------------------------------------
# Test modules mirror the `kernel/` directory layout:
#
#     test/unit/kernel/comm/strtoul/  -> tests kernel/comm/src/strtoul.c
#     test/unit/kernel/struct/dlink/   -> tests kernel/struct/double_link/src/dlink.c
#     test/unit/kernel/struct/sortlink/ -> tests kernel/struct/sortlink/src/sortlink.c
#
# Every test module sits in a leaf directory that contains exactly one
# CMakeLists.txt and a test_<name>.cpp. The leaf directory's basename
# is the module label (e.g. `dlink`) and the executable name follows
# the `test_<label>` convention.
#
# OSZ_TEST_MODULES controls which modules to load:
#   `all` (default)    every leaf directory with a CMakeLists.txt
#   `none`             no modules
#   `mod1;mod2`        semicolon-separated whitelist of leaf directory
#                      basenames
# Modules are loaded in alphabetical order so the ctest output is
# deterministic across runs.

# Recursively find every CMakeLists.txt under unit/, skipping this
# file itself. file(GLOB_RECURSE) is acceptable here because test
# modules are added by hand and we re-run cmake to pick up changes.
file(GLOB_RECURSE _cl_files
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
    LIST_DIRECTORIES false)
# file(GLOB_RECURSE) is greedy; strip anything that isn't a real test
# module dir.
set(_all_dirs)
foreach(_f ${_cl_files})
    get_filename_component(_dir "${_f}" DIRECTORY)
    # Skip this very file (test/unit/CMakeLists.txt).
    if("${_dir}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
        continue()
    endif()
    file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_dir}")
    list(APPEND _all_dirs "${_rel}")
endforeach()
list(SORT _all_dirs)

# Build a (label -> relative_dir) map. The label is the leaf dir's
# basename; collisions are an error (suggests two modules with the
# same label, which ctest can't disambiguate).
set(_labels)
set(_paths)
foreach(_rel ${_all_dirs})
    get_filename_component(_leaf "${_rel}" NAME)
    list(FIND _labels "${_leaf}" _idx)
    if(_idx GREATER -1)
        message(FATAL_ERROR
            "Duplicate test module label '${_leaf}' "
            "(found at ${_paths}; new path ${_rel}). "
            "Each test module's leaf directory name must be unique.")
    endif()
    list(APPEND _labels "${_leaf}")
    list(APPEND _paths "${_rel}")
endforeach()

# Honour OSZ_TEST_MODULES.
set(_raw "${OSZ_TEST_MODULES}")
if("${_raw}" STREQUAL "")
    set(_raw "all")
endif()

set(_selected_paths)
if("${_raw}" STREQUAL "all")
    set(_selected_paths "${_paths}")
elseif("${_raw}" STREQUAL "none")
    set(_selected_paths "")
else()
    set(_selected_paths "")
    foreach(_mod ${_raw})
        list(FIND _labels "${_mod}" _idx)
        if(_idx EQUAL -1)
            message(FATAL_ERROR
                "OSZ_TEST_MODULES: no test module labelled '${_mod}' "
                "(known labels: ${_labels}).")
        endif()
        list(GET _paths "${_idx}" _p)
        list(APPEND _selected_paths "${_p}")
    endforeach()
endif()

list(SORT _selected_paths)

# Pair the selected paths back with their labels for a friendly status
# message.
set(_selected_labels)
foreach(_p ${_selected_paths})
    get_filename_component(_leaf "${_p}" NAME)
    list(APPEND _selected_labels "${_leaf}")
endforeach()
message(STATUS "OSZ host test modules: ${_selected_labels}")
set(OSZ_TEST_MODULES_SELECTED "${_selected_labels}"
    CACHE INTERNAL "Resolved list of test modules added in this build.")

foreach(_p ${_selected_paths})
    add_subdirectory("${_p}")
endforeach()