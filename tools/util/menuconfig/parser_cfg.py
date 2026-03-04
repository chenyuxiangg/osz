#!/usr/bin/env python3
"""
Parse .config file and generate header and Makefile variables
"""

import os
import sys
import re

# Script is located at: <project_root>/tools/util/menuconfig/parser_cfg.py
# We need to compute paths relative to the script location
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
# Project root is three levels up: tools/util/menuconfig -> tools/util -> tools -> project_root
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(SCRIPT_DIR)))
CONSTRUCT_DIR = os.path.join(PROJECT_ROOT, "construct")

# Derived paths
MAKE_PATH = os.path.join(CONSTRUCT_DIR, "make")
CMAKE_PATH = os.path.join(CONSTRUCT_DIR, "cmake")
TARGETS_PATH = os.path.join(PROJECT_ROOT, "targets")
TARGET_NAME = sys.argv[1] if len(sys.argv) >= 2 else "osz"
INCLUDE_NAME = "menuconfig.h"
CONFIG_MK_NAME = "cfg.mk"
CONFIG_CMAKE_NAME = "cfg.cmake"

def parse_config_file(cfg_abs_path):
    """
    Parse .config file (Kconfig format)
    Returns dict with config keys and values
    """
    cfg_info = dict()
    
    if not os.path.exists(cfg_abs_path):
        print(f"Error: Config file {cfg_abs_path} not found")
        sys.exit(1)
    
    with open(cfg_abs_path, "r") as cfg_f:
        lines = cfg_f.readlines()
        
    for line in lines:
        line = line.strip()
        
        # Skip empty lines and comments
        if not line or line.startswith('#'):
            continue
        
        # Handle .config format: CONFIG_XXX=y or CONFIG_XXX=value
        if line.startswith('CONFIG_'):
            # Handle "is not set" format
            if line.startswith('# CONFIG_') and ' is not set' in line:
                match = re.match(r'^# CONFIG_([A-Za-z0-9_]+) is not set$', line)
                if match:
                    key = match.group(1)
                    cfg_info[key] = 0  # Use 0 for 'n'
                continue
            
            # Parse CONFIG_XXX=value
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                
                # Remove CONFIG_ prefix
                if key.startswith('CONFIG_'):
                    key = key[7:]  # Remove 'CONFIG_'
                
                # Handle string values with quotes
                if value.startswith('"') and value.endswith('"'):
                    value = value[1:-1]
                
                # Convert to appropriate type
                if value == 'y':
                    cfg_info[key] = 1
                elif value == 'n':
                    cfg_info[key] = 0
                elif re.match(r'^0x[0-9a-fA-F]+$', value):
                    # Hexadecimal
                    cfg_info[key] = value
                elif value.isdigit():
                    # Decimal integer
                    cfg_info[key] = int(value)
                else:
                    # String
                    cfg_info[key] = value
    
    return cfg_info


def gen_cfgmk(cfg_info, cfgmk_abs_path):
    """Generate cfg.mk file from config info"""
    if cfg_info:
        with open(cfgmk_abs_path, "w") as mk_f:
            for k, v in cfg_info.items():
                if v == 1:
                    mk_f.write(f"{k} := y\n")
                elif v == 0:
                    mk_f.write(f"{k} := n\n")
                elif isinstance(v, str):
                    mk_f.write(f"{k} := {v}\n")
                else:
                    mk_f.write(f"{k} := {v}\n")
            
            mk_f.write("\nOSZ_CFG_MACRO := ")
            for k, v in cfg_info.items():
                if v == 1:
                    mk_f.write(f"-D{k} ")
                elif isinstance(v, str):
                    mk_f.write(f"-D{k}={v} ")
                elif v != 0:  # Skip n values
                    mk_f.write(f"-D{k}={v} ")


def gen_cfgcmk(cfg_info, cfgcmk_abs_path):
    """Generate cfg.cmake file from config info"""
    if cfg_info:
        with open(cfgcmk_abs_path, "w") as cmk_f:
            for k, v in cfg_info.items():
                if v == 1:
                    cmk_f.write(f"set({k} y)\n")
                elif v == 0:
                    cmk_f.write(f"set({k} n)\n")
                elif isinstance(v, str):
                    cmk_f.write(f"set({k} {v})\n")
                else:
                    cmk_f.write(f"set({k} {v})\n")
            
            cmk_f.write("\nset(OSZ_CFG_MACRO \n")
            for k, v in cfg_info.items():
                if v == 1:
                    cmk_f.write(f"{k}=1\n")
                elif isinstance(v, str):
                    cmk_f.write(f"{k}={v}\n")
                elif v != 0:
                    cmk_f.write(f"{k}={v}\n")
            cmk_f.write("\n)")


def gen_include(cfg_info, include_abs_path):
    """Generate C header file from config info"""
    if cfg_info:
        with open(include_abs_path, "w") as mk_f:
            for k, v in cfg_info.items():
                if v == 1:
                    mk_f.write(f"#define {k}\t1\n")
                elif v == 0:
                    mk_f.write(f"// #define {k}\t0\n")
                elif isinstance(v, str):
                    mk_f.write(f"#define {k}\t{v}\n")
                else:
                    mk_f.write(f"#define {k}\t{v}\n")


def main():
    """Main function"""
    print("====== Start Generate Configuration ======")
    
    # Parse .config file - now located in the same directory as this script
    config_file = os.path.join(SCRIPT_DIR, ".config")
    if not os.path.exists(config_file):
        print(f"Error: .config file not found at {config_file}")
        print("Please run 'make menuconfig' first to generate configuration.")
        sys.exit(1)
    
    cfg_info = parse_config_file(config_file)
    
    if not cfg_info:
        print(f"Error: No valid configuration found in {config_file}")
        sys.exit(1)
    
    print(f"Parsed {len(cfg_info)} configuration items from {config_file}")
    
    # Generate outputs
    gen_cfgmk(cfg_info, os.path.join(MAKE_PATH, CONFIG_MK_NAME))
    gen_cfgcmk(cfg_info, os.path.join(CMAKE_PATH, CONFIG_CMAKE_NAME))
    gen_include(cfg_info, os.path.join(TARGETS_PATH, r"platform/include/{}".format(INCLUDE_NAME)))
    
    print("====== End Generate Configuration ======")


if __name__ == "__main__":
    main()
