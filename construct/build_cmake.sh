#!/bin/bash

# build_cmake.sh - Build script for OSZ using CMake
# Usage: ./build_cmake.sh [target1] [target2] ...
#        ./build_cmake.sh --help   # Show help and list available targets

# add new custom target:
#   mudify 'case "$target" in' in show_help function, for example:
#
#   my_new_target)
#    desc="用户自定义的描述"
#    ;;


set -e  # Exit on error

# Function to show help
show_help() {
    cat <<EOF
Usage: $0 [OPTIONS] [TARGETS...]

Build targets using CMake. If no targets are specified, builds the default target (all).

Options:
  menuconfig        Run interactive configuration interface (Kconfig)
  genconfig         Generate default configuration
  --help            Show this help message and list available targets

Examples:
  $0                       # Build all targets
  $0 target1               # Build target1 only
  $0 target1 target2       # Build target1 and target2
  $0 menuconfig            # Run configuration interface
  $0 genconfig             # Generate default configuration

EOF
    # Ensure we are in the build directory and CMake is configured
    current_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    project_root=$(cd "${current_dir}/.." && pwd)
    build_dir="${project_root}/build"
    
    # If build directory doesn't exist, create it
    if [ ! -d "${build_dir}" ]; then
        mkdir -p "${build_dir}"
    fi
    
    # If CMakeCache.txt doesn't exist, run cmake configuration
    if [ ! -f "${build_dir}/CMakeCache.txt" ]; then
        cd "${build_dir}"
        cmake "${project_root}" >/dev/null 2>&1
    else
        cd "${build_dir}"
    fi
    
    # List available targets using cmake --build . --target help
    # We'll parse the output and add descriptions for known targets
    echo "Available targets (with descriptions):"
    echo ""
    
    # Use a temporary file to capture output since while loop in pipeline runs in subshell
    temp_file=$(mktemp)
    (cd "$build_dir" && cmake --build . --target help > "$temp_file" 2>&1)
    
    # Read the file line by line
    while IFS= read -r line; do
        # Skip lines that are not target lines or are empty
        # Pattern matches lines starting with "... target_name (optional description in parentheses)"
        # Target name may contain slashes (e.g., install/local)
        if [[ "$line" =~ ^\.\.\.\ ([a-zA-Z0-9_/][a-zA-Z0-9_/-]*)(\ \((.*)\))?$ ]]; then
            target="${BASH_REMATCH[1]}"
            cmake_desc="${BASH_REMATCH[3]:-}"
            
            # Add custom descriptions based on target name patterns
            case "$target" in
                all)
                    desc="Default target, builds everything"
                    ;;
                clean)
                    desc="Clean build artifacts"
                    ;;
                target_qemu_all)
                    desc="Build QEMU virtual platform executable (complete build)"
                    ;;
                target_qemu)
                    desc="QEMU virtual platform executable"
                    ;;
                target_qemu_run)
                    desc="Build and run QEMU virtual platform"
                    ;;
                target_qemu_debug)
                    desc="Build for debugging with GDB"
                    ;;
                target_qemu_disasm)
                    desc="Generate disassembly files"
                    ;;
                arch_*)
                    desc="Architecture-specific module: $(echo "$target" | sed 's/arch_//')"
                    ;;
                k_base_*)
                    base_module=$(echo "$target" | sed 's/k_base_//')
                    case "$base_module" in
                        hook)   mod_desc="System hook functions" ;;
                        init)   mod_desc="Kernel initialization" ;;
                        ipc)    mod_desc="Inter-process communication" ;;
                        mem)    mod_desc="Memory management" ;;
                        sche)   mod_desc="Task scheduler" ;;
                        swt)    mod_desc="Software timer" ;;
                        task)   mod_desc="Task management" ;;
                        trap)   mod_desc="Exception and trap handling" ;;
                        *)      mod_desc="$base_module module" ;;
                    esac
                    desc="Kernel base module: $mod_desc"
                    ;;
                k_*)
                    kernel_module=$(echo "$target" | sed 's/k_//')
                    case "$kernel_module" in
                        comm)   mod_desc="Common utilities and definitions" ;;
                        debug)  mod_desc="Debugging facilities" ;;
                        dfx)    mod_desc="Diagnostic and fault extension" ;;
                        drv_*)  mod_desc="Driver: $(echo "$kernel_module" | sed 's/drv_//')" ;;
                        shell)  mod_desc="System shell" ;;
                        startup) mod_desc="Kernel startup code" ;;
                        struct_*) mod_desc="Data structure: $(echo "$kernel_module" | sed 's/struct_//')" ;;
                        *)      mod_desc="$kernel_module module" ;;
                    esac
                    desc="Kernel module: $mod_desc"
                    ;;
                edit_cache)
                    desc="Edit CMake cache variables"
                    ;;
                rebuild_cache)
                    desc="Rebuild CMake cache"
                    ;;
                depend)
                    desc="Generate dependencies"
                    ;;
                install)
                    desc="Install all targets"
                    ;;
                install/local)
                    desc="Install only local targets (not from dependencies)"
                    ;;
                install/strip)
                    desc="Install targets with stripping (reduce size)"
                    ;;
                list_install_components)
                    desc="List installable components"
                    ;;
                *)
                    # Use CMake's description if available, otherwise generate based on name patterns
                    if [ -n "$cmake_desc" ]; then
                        desc="$cmake_desc"
                    else
                        # Generate description based on target name patterns
                        case "$target" in
                            test_*|*_test)
                                desc="Test target"
                                ;;
                            demo_*|*_demo)
                                desc="Demo or example target"
                                ;;
                            example_*)
                                desc="Example target"
                                ;;
                            bench_*|*_bench|*_benchmark)
                                desc="Benchmark target"
                                ;;
                            *_ut|ut_*)
                                desc="Unit test target"
                                ;;
                            *_st|st_*)
                                desc="System test target"
                                ;;
                            *_app|app_*)
                                desc="Application target"
                                ;;
                            *_lib|lib_*)
                                desc="Library target"
                                ;;
                            *_drv|drv_*)
                                desc="Driver target"
                                ;;
                            *_hal|hal_*)
                                desc="Hardware abstraction layer target"
                                ;;
                            *_sample|sample_*)
                                desc="Sample code target"
                                ;;
                            *)
                                # For targets with slashes, provide more specific description
                                if [[ "$target" == */* ]]; then
                                    base_target="${target##*/}"
                                    parent_dir="${target%/*}"
                                    desc="Target '$base_target' in '$parent_dir' category"
                                else
                                    desc="Custom target"
                                fi
                                ;;
                        esac
                    fi
                    ;;
            esac
            
            printf "  %-30s - %s\n" "$target" "$desc"
        fi
    done < "$temp_file"
    
    rm -f "$temp_file"
    
    echo ""
    echo "For more details on a specific target, run: cmake --build . --target help"
}

# Function to run menuconfig
run_menuconfig() {
    echo "Running menuconfig..."
    current_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    project_root=$(cd "${current_dir}/.." && pwd)
    cd "${project_root}"
    
    # Check if we're in a terminal that supports interactive mode
    if [ ! -t 0 ] || [ ! -t 1 ]; then
        echo "Warning: Not in an interactive terminal. Trying to run menuconfig anyway..."
    fi
    
    # Set terminal type if not set
    if [ -z "$TERM" ]; then
        export TERM=xterm-256color
    fi
    
    # Config file path
    menuconfig_config="tools/util/menuconfig/.config"
    
    # First, try to use system 'menuconfig' command if available
    if command -v menuconfig >/dev/null 2>&1; then
        echo "Using system menuconfig command..."
        KCONFIG_CONFIG="${menuconfig_config}" srctree="." menuconfig enter.kconfig 2>&1
    else
        # Otherwise, use python kconfiglib module
        echo "Using python kconfiglib module..."
        # Check if kconfiglib is available
        python3 -c "import kconfiglib" 2>/dev/null
        if [ $? -ne 0 ]; then
            echo "Installing kconfiglib..."
            pip3 install kconfiglib
        fi
        
        # Run menuconfig using the existing Kconfig entry point
        python3 -m kconfiglib menuconfig enter.kconfig --config "${menuconfig_config}" 2>&1
    fi
    
    # Check if configuration was updated
    if [ -f "${menuconfig_config}" ]; then
        echo "Configuration saved to ${menuconfig_config}"
        echo "Regenerating cfg.cmake..."
        python3 tools/util/menuconfig/parser_cfg.py
    else
        echo "No configuration changes made."
    fi
}

# Function to generate default configuration
gen_defconfig() {
    echo "Generating default configuration..."
    current_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    project_root=$(cd "${current_dir}/.." && pwd)
    cd "${project_root}"
    
    # Check if kconfiglib is available
    python3 -c "import kconfiglib" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "Installing kconfiglib..."
        pip3 install kconfiglib
    fi
    
    # Generate default config using the existing script, output to tools/util/menuconfig/.config
    config_file="tools/util/menuconfig/.config"
    python3 tools/util/menuconfig/gen_defconfig.py enter.kconfig "${config_file}"
    # Then parse it to generate cfg.cmake and others
    python3 tools/util/menuconfig/parser_cfg.py
}

# Function to load configuration
load_config() {
    current_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
    project_root=$(cd "${current_dir}/.." && pwd)
    config_file="${project_root}/tools/util/menuconfig/.config"
    
    # If .config doesn't exist, generate default config
    if [ ! -f "${config_file}" ]; then
        echo "No configuration found. Generating default configuration..."
        gen_defconfig
    fi
    
    # Check if cfg.cmake is up to date, if not, parse .config
    cfg_cmake="${project_root}/construct/cmake/cfg.cmake"
    if [ ! -f "${cfg_cmake}" ] || [ "${config_file}" -nt "${cfg_cmake}" ]; then
        echo "Configuration changed. Regenerating cfg.cmake..."
        python3 tools/util/menuconfig/parser_cfg.py
    fi
}

# Check for menuconfig option
if [[ "$1" == "menuconfig" ]]; then
    run_menuconfig
    exit 0
fi

# Check for genconfig option
if [[ "$1" == "genconfig" ]]; then
    gen_defconfig
    exit 0
fi

# Check for help option
if [[ "$1" == "--help" ]]; then
    show_help
    exit 0
fi

# Load configuration before building
load_config

current_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
project_root=$(cd "${current_dir}/.." && pwd)
build_dir="${project_root}/build"

echo "Project root: ${project_root}"
echo "Build directory: ${build_dir}"

# Create build directory if it doesn't exist
if [ ! -d "${build_dir}" ]; then
    echo "Creating build directory..."
    mkdir -p "${build_dir}"
fi

# Check if CMakeLists.txt exists
if [ ! -f "${project_root}/CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found in ${project_root}" >&2
    exit 1
fi

# Determine if we need to run cmake configuration
# We run cmake if:
# 1. build directory is empty, or
# 2. CMakeCache.txt doesn't exist, or
# 3. CMakeLists.txt is newer than CMakeCache.txt
need_reconfigure=0
if [ ! -f "${build_dir}/CMakeCache.txt" ]; then
    need_reconfigure=1
elif [ "${project_root}/CMakeLists.txt" -nt "${build_dir}/CMakeCache.txt" ]; then
    need_reconfigure=1
fi

# Run cmake configuration if needed
if [ ${need_reconfigure} -eq 1 ]; then
    echo "Running CMake configuration..."
    cd "${build_dir}"
    cmake "${project_root}"
else
    echo "Using existing CMake configuration."
    cd "${build_dir}"
fi

# Build targets
if [ $# -eq 0 ]; then
    echo "Building default target (all)..."
    cmake --build . -- -j$(nproc)
else
    for target in "$@"; do
        echo "Building target: ${target}..."
        cmake --build . --target "${target}" -- -j$(nproc)
    done
fi

echo "Build completed successfully."
