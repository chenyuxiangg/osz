# set current file env path
COMM_CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))

# set compiler info
G_COMPILER_NAME := riscv32-unknown-elf
CROSS = $(CROSS_COMPILER_PATH)/$(G_COMPILER_NAME)-
CC = $(CROSS)gcc
LD = $(CROSS)ld
AR = $(CROSS)ar
OBJDUMP = $(CROSS)objdump
OBJCOPY = $(CROSS)objcopy
GDB := $(CROSS)gdb

# set global env path, every mk file can use
G_PROJECT_PATH := $(dir $(COMM_CURRENT_PATH))/../..
G_KERNEL_PATH := $(G_PROJECT_PATH)/kernel
G_BUILD_PATH := $(G_PROJECT_PATH)/build
G_TEST_PATH := $(G_PROJECT_PATH)/test
G_ARCH_PATH := $(G_PROJECT_PATH)/arch
G_OUTPUT_PATH := $(G_PROJECT_PATH)/output
G_PLATFORM_PATH := $(G_PROJECT_PATH)/platform
G_TARGETS_PATH := $(G_PROJECT_PATH)/targets
G_OPENSOURCE_PATH := $(G_PROJECT_PATH)/opensource
G_KERNEL_DEBUG_PATH := $(G_KERNEL_PATH)/debug
G_KERNEL_COMM_PATH := $(G_KERNEL_PATH)/comm
G_KERNEL_BASE_PATH := $(G_KERNEL_PATH)/base
G_KERNEL_DRIVERS_PATH := $(G_KERNEL_PATH)/drivers

# set hide command
HIDE := @

# set linker file
OSZ_LD := $(G_BUILD_PATH)/osz.ld

# some comm function
define build_module_target
	$(HIDE)echo "[Start]	Processing in $(G_PROJECT_PATH)/$(1)"
	$(HIDE)echo "[Make]	Make for $(basename $(notdir $(1))).a"
	$(HIDE)$(MAKE) -C $(G_PROJECT_PATH)/$(1) all
	$(HIDE)echo "[End]	build Successful for lib$(basename $(notdir $(1))).a"
	
endef

define add_libs
	ifeq ($(OSZ_CFG_$(shell echo $(1) | tr a-z A-Z)), y)
	LIBS += -l$(1)
	endif
endef

define add_module_config
    ifneq ($$(wildcard $$(G_PROJECT_PATH)/$(1)/config.mk),)
    include $$(G_PROJECT_PATH)/$(1)/config.mk
    OSZ_INCLUDE += $$(LOCAL_$(notdir $(1))_INCLUDE)
    endif
endef

include $(CFG_MK)

# global module
MODULES_y :=
MODULES_n :=
MODULES_$(OSZ_CFG_STARTUP) += kernel/startup
MODULES_$(OSZ_CFG_STRUCT) += kernel/base/struct
MODULES_$(OSZ_CFG_COMM) += kernel/comm
MODULES_$(OSZ_CFG_MEM) += kernel/base/mem
MODULES_$(OSZ_CFG_SHELL) += kernel/shell
MODULES_$(OSZ_CFG_UART) += kernel/drivers/uart
MODULES_$(OSZ_CFG_TIMER) += kernel/drivers/timer
MODULES_$(OSZ_CFG_TICK) += kernel/drivers/tick
MODULES_$(OSZ_CFG_DEBUG) += kernel/debug
MODULES_$(OSZ_CFG_SCHEDULE) += kernel/base/schedule
MODULES_$(OSZ_CFG_TASK) += kernel/base/task
MODULES_$(OSZ_CFG_IPC) += kernel/base/ipc
MODULES_$(OSZ_CFG_TRAP) += kernel/base/trap
MODULES_$(OSZ_CFG_INIT) += kernel/base/init
MODULES_$(OSZ_CFG_RISCV) += arch/riscv
MODULES_$(OSZ_CFG_TEST) += test
MODULES_$(OSZ_CFG_TARGETS) += targets

# global compile macro
OSZ_INCLUDE := -I$(G_PLATFORM_PATH)/$(OSZ_CFG_PLATFORM)/include
OSZ_INCLUDE += -I$(G_TARGETS_PATH)/$(OSZ_CFG_PLATFORM)/include
OSZ_INCLUDE += -I$(G_TEST_PATH)/
$(foreach mod,$(MODULES_y),$(eval $(call add_module_config,$(mod))))