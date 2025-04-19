CONFIG_CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CONFIG_CURRENT_DIR := $(dir $(CONFIG_CURRENT_PATH))
CONFIG_MODULE := riscv

LOCAL_$(CONFIG_MODULE)_INCLUDE :=
ifeq ($(OSZ_CFG_RISCV), y)
LOCAL_$(CONFIG_MODULE)_INCLUDE += -I$(CONFIG_CURRENT_DIR)/include
endif