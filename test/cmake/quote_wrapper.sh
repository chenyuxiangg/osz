#!/bin/bash
#
# quote_wrapper.sh — compiler launcher that rewrites a consolidated
# `-iquote dir1 dir2 ...` flag into one `-iquote <dir>` per directory.
#
# Why: CMake's Unix Makefiles (and Ninja) generators consolidate
# consecutive `-iquote` flags into a single one with multiple paths.
# GCC's `-iquote` only honours the FIRST path in such a flag, so every
# directory after the first stops being searchable for `#include "..."`.
# This wrapper splits the consolidated flag back apart before invoking
# the real compiler.
#
# Invocation:
#     <wrapper> <compiler> <flags...>    -- same as running the compiler directly
#
# In compile mode (a `-c`, `-S`, or `-E` flag is present) we expand
# `-iquote` arguments. In any other mode (notably the link step) we
# pass everything through unchanged, because the argument shape is
# different and the rewrite isn't needed.

set -euo pipefail

COMPILER="${1:-}"
if [ -z "$COMPILER" ]; then
    echo "quote_wrapper.sh: missing compiler argument" >&2
    exit 1
fi
shift

# Detect compile mode. If we see -c, -S, or -E, we know this is a
# compilation / preprocessing invocation. Otherwise we treat it as
# linking and pass through.
COMPILE_MODE=0
for arg in "$@"; do
    case "$arg" in
        -c|-S|-E|-M|-MM|-MD|-MMD) COMPILE_MODE=1; break ;;
    esac
done

if [ "$COMPILE_MODE" -eq 0 ]; then
    exec "$COMPILER" "$@"
fi

# Compile mode: walk the arg list, capture every "-iquote <list>" pair,
# and rebuild the rest.
QUOTE_LIST=""
PASSTHROUGH=()

i=1   # $1 is the first real argument; $0 is the script path.
n=$#
while [ "$i" -le "$n" ]; do
    arg="${!i}"
    i=$((i + 1))

    if [ "$arg" = "-iquote" ]; then
        # Consume following non-flag args as part of the directory list.
        new_dirs=""
        while [ "$i" -le "$n" ]; do
            nxt="${!i}"
            case "$nxt" in
                -*) break ;;
                *)
                    if [ -z "$new_dirs" ]; then
                        new_dirs="$nxt"
                    else
                        new_dirs="$new_dirs $nxt"
                    fi
                    i=$((i + 1))
                    ;;
            esac
        done
        if [ -n "$new_dirs" ]; then
            if [ -z "$QUOTE_LIST" ]; then
                QUOTE_LIST="$new_dirs"
            else
                QUOTE_LIST="$QUOTE_LIST $new_dirs"
            fi
        fi
    else
        PASSTHROUGH+=("$arg")
    fi
done

# Emit one -iquote per directory, then the rest of the args.
EXTRA=()
if [ -n "$QUOTE_LIST" ]; then
    # shellcheck disable=SC2206
    for d in $QUOTE_LIST; do
        EXTRA+=("-iquote" "$d")
    done
fi

exec "$COMPILER" "${EXTRA[@]}" "${PASSTHROUGH[@]}"
