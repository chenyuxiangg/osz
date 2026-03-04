#!/usr/bin/env python3
"""
Generate default configuration from Kconfig
"""

import sys
import os

try:
    import kconfiglib
except ImportError:
    print("Error: kconfiglib module not found.")
    print("Please install with: pip install kconfiglib")
    sys.exit(1)

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <Kconfig> <output.config>")
        sys.exit(1)
    
    kconfig_file = os.path.abspath(sys.argv[1])
    output_file = os.path.abspath(sys.argv[2])
    
    if not os.path.exists(kconfig_file):
        print(f"Error: Kconfig file '{kconfig_file}' not found")
        sys.exit(1)
    
    try:
        # Load Kconfig and write default configuration
        srctree = os.path.dirname(kconfig_file)
        os.environ['srctree'] = srctree
        kconf = kconfiglib.Kconfig(kconfig_file)
        kconf.write_config(output_file)
        print(f"Default configuration written to {output_file}")
    except Exception as e:
        print(f"Error generating configuration: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
