#!/usr/bin/env bash
#
# runtest.sh - one-shot script: configure + build + run the host-side OSZ
# unit tests under `test/`. The tests live in a separate CMake project that
# uses the HOST (gcc/g++) toolchain, NOT the RISC-V cross toolchain.
#
# Usage:
#     ./test/runtest.sh                       # configure + build + run all
#     ./test/runtest.sh --clean               # wipe test/build first
#     ./test/runtest.sh --rebuild             # clean + full rebuild
#     ./test/runtest.sh --build               # only build, don't run
#     ./test/runtest.sh --run                 # only run (skip build)
#
# Module / test-case selection (works with any action above):
#     --module <name>[,<name>...]   # ctest labels to INCLUDE  (repeatable)
#     --exclude-module <name>[,...] # ctest labels to EXCLUDE  (repeatable)
#     --gtest-filter <pattern>      # pass --gtest_filter=<pattern> to gtest
#                                   # binaries directly (skips ctest)
#
# Examples:
#     ./test/runtest.sh --module dlink,strtoul
#     ./test/runtest.sh --run --module sortlink
#     ./test/runtest.sh --module dlink --module strtoul
#     ./test/runtest.sh --module dlink --exclude-module sortlink
#     ./test/runtest.sh --run --gtest-filter 'DLink.Init*'
#
# Module names are the ctest labels (lower-case, no 'test_' prefix):
#     dlink, sortlink, strtoul
#
# Selection mode trade-offs:
#   - --module / --exclude-module -> runs via ctest with label filtering.
#     Best for whole-module runs, integrates with ctest's reporting.
#   - --gtest-filter -> runs binaries directly with --gtest_filter.
#     Best for fine-grained single-test selection within a module.
#     Can be combined with --module / --exclude-module to narrow binaries.
#

set -euo pipefail

# ---- Locate project root ---------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
# Build directory lives INSIDE test/ so the project root stays clean.
BUILD_DIR="${SCRIPT_DIR}/build"
BIN_DIR="${BUILD_DIR}/bin"

# ---- Parse options --------------------------------------------------------
ACTION="all"            # one of: all, build, run, rebuild, clean
MODULES=()              # ctest labels to INCLUDE
EXCLUDE_MODULES=()      # ctest labels to EXCLUDE
GTEST_FILTER=""         # if non-empty, run gtest binaries directly with this filter

usage() {
    sed -n '2,40p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

i=1
n=$#
while [ "$i" -le "$n" ]; do
    arg="${!i}"
    i=$((i + 1))

    case "${arg}" in
        --clean)   ACTION="clean" ;;
        --rebuild) ACTION="rebuild" ;;
        --build)   ACTION="build" ;;
        --run)     ACTION="run" ;;
        --module)
            mods=""
            while [ "$i" -le "$n" ]; do
                nxt="${!i}"
                case "${nxt}" in
                    --*) break ;;
                    *)  mods="${mods:+${mods},}${nxt}"; i=$((i + 1)) ;;
                esac
            done
            if [ -z "${mods}" ]; then
                echo "--module requires a comma-separated list of module names" >&2
                exit 2
            fi
            IFS=',' read -ra _parts <<< "${mods}"
            MODULES+=("${_parts[@]}")
            unset _parts
            ;;
        --exclude-module)
            mods=""
            while [ "$i" -le "$n" ]; do
                nxt="${!i}"
                case "${nxt}" in
                    --*) break ;;
                    *)  mods="${mods:+${mods},}${nxt}"; i=$((i + 1)) ;;
                esac
            done
            if [ -z "${mods}" ]; then
                echo "--exclude-module requires a comma-separated list of module names" >&2
                exit 2
            fi
            IFS=',' read -ra _parts <<< "${mods}"
            EXCLUDE_MODULES+=("${_parts[@]}")
            unset _parts
            ;;
        --gtest-filter)
            if [ "$i" -gt "$n" ]; then
                echo "--gtest-filter requires a pattern argument" >&2
                exit 2
            fi
            GTEST_FILTER="${!i}"
            i=$((i + 1))
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: ${arg}" >&2
            usage >&2
            exit 2
            ;;
    esac
done

cd "${PROJECT_ROOT}"

configure() {
    echo "==> Configuring host test build in ${BUILD_DIR}"
    cmake -S test -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-O0 -g"
}

build() {
    echo "==> Building host tests"
    cmake --build "${BUILD_DIR}" -j "$(nproc 2>/dev/null || echo 2)"
}

# Build ctest -L / -LE filter args from MODULES / EXCLUDE_MODULES. Each
# module name is a ctest LABEL.
ctest_filter_args() {
    local args=()
    if [ "${#MODULES[@]}" -gt 0 ]; then
        local joined=""
        for m in "${MODULES[@]}"; do
            joined="${joined:+${joined}|}${m}"
        done
        args+=(-L "${joined}")
    fi
    if [ "${#EXCLUDE_MODULES[@]}" -gt 0 ]; then
        local joined=""
        for m in "${EXCLUDE_MODULES[@]}"; do
            joined="${joined:+${joined}|}${m}"
        done
        args+=(-LE "${joined}")
    fi
    printf '%s\n' "${args[@]}"
}

# Resolve module labels (or "all" when empty) to a list of binary basenames
# ("test_<label>") that exist in BIN_DIR. Dies loudly if a label has no
# matching binary.
modules_to_binaries() {
    local labels=("$@")
    local bins=()
    if [ "${#labels[@]}" -eq 0 ]; then
        for b in "${BIN_DIR}"/test_*; do
            [ -x "${b}" ] || continue
            bins+=("$(basename "${b}")")
        done
    else
        for lbl in "${labels[@]}"; do
            local b="${BIN_DIR}/test_${lbl}"
            if [ ! -x "${b}" ]; then
                echo "No test binary for module '${lbl}' (looked for ${b})" >&2
                exit 1
            fi
            bins+=("test_${lbl}")
        done
    fi
    printf '%s\n' "${bins[@]}"
}

run() {
    if [ -n "${GTEST_FILTER}" ]; then
        # Direct gtest invocation. --module still narrows which binaries
        # we call; --exclude-module is honoured too.
        # shellcheck disable=SC2207
        include_bins=( $(modules_to_binaries "${MODULES[@]}") )
        local exclude_set=" ${EXCLUDE_MODULES[*]} "
        echo "==> Running gtest binaries directly (filter='${GTEST_FILTER}')"
        echo "    binaries: ${include_bins[*]}"
        local overall=0
        for bin in "${include_bins[@]}"; do
            local label="${bin#test_}"
            if [[ "${exclude_set}" == *" ${label} "* ]]; then
                echo "    skipping ${bin} (excluded)"
                continue
            fi
            echo "    -- ${bin}"
            if ! "${BIN_DIR}/${bin}" --gtest_filter="${GTEST_FILTER}"; then
                overall=1
            fi
        done
        exit "${overall}"
    fi

    echo "==> Running tests via ctest"
    local filter_args
    # shellcheck disable=SC2207
    filter_args=( $(ctest_filter_args) )
    if [ "${#MODULES[@]}" -gt 0 ] || [ "${#EXCLUDE_MODULES[@]}" -gt 0 ]; then
        echo "    include modules: ${MODULES[*]:-<none>}"
        echo "    exclude modules: ${EXCLUDE_MODULES[*]:-<none>}"
    fi
    ctest --test-dir "${BUILD_DIR}" --output-on-failure \
        "${filter_args[@]}"
}

clean() {
    echo "==> Removing ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
}

case "${ACTION}" in
    clean)
        clean
        ;;
    rebuild)
        clean
        configure
        build
        run
        ;;
    build)
        configure
        build
        ;;
    run)
        run
        ;;
    all)
        configure
        build
        run
        ;;
esac

echo "==> Done."