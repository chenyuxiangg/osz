#!/bin/bash

# Determine paths relative to this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONSTRUCT_DIR="$SCRIPT_DIR"
PROJECT_ROOT="$(dirname "$CONSTRUCT_DIR")"

# Source environment configuration
source "$CONSTRUCT_DIR/script/env.sh"

params=$@

pre_compiler() {
    rm -rf ${OUTPUT_PATH}
}

pre_compiler
for param in ${params[@]}
do
    make -C "$CONSTRUCT_DIR" ${param}
done
