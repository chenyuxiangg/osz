import os
import sys

PROJECT_PATH = os.path.dirname(os.getcwd())
BUILD_PATH = os.path.join(PROJECT_PATH, "construct")
SCRIPT_PATH = os.path.join(BUILD_PATH, "script")
CONFIG_PATH = os.path.join(BUILD_PATH, "config")
MAKE_PATH = os.path.join(BUILD_PATH, "make")
CMAKE_PATH = os.path.join(BUILD_PATH, "cmake")
TARGETS_PATH = os.path.join(PROJECT_PATH, "targets")
TARGET_NAME = sys.argv[1] if len(sys.argv) >= 2 else "osz"
INCLUDE_NAME = "menuconfig.h"
CONFIG_MK_NAME = "cfg.mk"
CONFIG_CMAKE_NAME = "cfg.cmake"

def parser_cfg(cfg_abs_path):
    cfg_info = dict()
    with open(cfg_abs_path, "r") as cfg_f:
        lines = cfg_f.readlines()
        for line in lines:
            line = line.strip()
            if line.find("#") != -1:
                continue
            if line.lower().find("osz") == -1:
                continue
            params = line.split("=")
            if params[1].lower() == "y":
                cfg_info[params[0]] = 1
            elif params[1].isdigit():
                cfg_info[params[0]] = int(params[1])
            elif params[1].lower() == "n":
                continue
            else:
                cfg_info[params[0]] = params[1]
    return cfg_info


def gen_cfgmk(cfg_info, cfgmk_abs_path):
    if cfg_info:
        with open(cfgmk_abs_path, "w") as mk_f:
            for k,v in cfg_info.items():
                if v == 1:
                    mk_f.write("{} := y\n".format(k))
                elif isinstance(v, str):
                    mk_f.write("{} := {}\n".format(k, v))
            mk_f.write("\nOSZ_CFG_MACRO := ")
            for k,v in cfg_info.items():
                if v == 1:
                    mk_f.write("-D{} ".format(k))
                elif isinstance(v, str):
                    mk_f.write("-D{}={} ".format(k, v))

def gen_cfgcmk(cfg_info, cfgcmk_abs_path):
    if cfg_info:
        with open(cfgcmk_abs_path, "w") as cmk_f:
            for k,v in cfg_info.items():
                if v == 1:
                    cmk_f.write("set({} y)\n".format(k))
                elif isinstance(v, str):
                    cmk_f.write("set({} {})\n".format(k, v))
            cmk_f.write("\nset(OSZ_CFG_MACRO \n")
            for k,v in cfg_info.items():
                if v == 1:
                    cmk_f.write("{}=1\n".format(k))
                elif isinstance(v, str):
                    cmk_f.write("{}={}\n".format(k, v))
            cmk_f.write("\n)")

def gen_include(cfg_info, include_abs_path):
    if cfg_info:
        with open(include_abs_path, "w") as mk_f:
            for k,v in cfg_info.items():
                mk_f.write("#define {}\t{}\n".format(k, v))
                

if __name__ == "__main__":
    print("====== Start Gen Cfg ======")
    cfg_info = parser_cfg(os.path.join(CONFIG_PATH, r"{}.config".format(TARGET_NAME)))
    gen_cfgmk(cfg_info, os.path.join(MAKE_PATH, CONFIG_MK_NAME))
    gen_cfgcmk(cfg_info, os.path.join(CMAKE_PATH, CONFIG_CMAKE_NAME))
    gen_include(cfg_info, os.path.join(TARGETS_PATH, r"platform/include/{}".format(INCLUDE_NAME)))
    print("====== End Gen Cfg ======")