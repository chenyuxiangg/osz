include ./make/comm.mk
include ./make/undefine_symb.mk

# Kconfig integration
KCONFIG_SRC = $(abspath ../enter.kconfig)
OUTPUT_CONFIG = $(abspath ../tools/util/menuconfig/.config)
SCRIPT_DIR = ../tools/util/menuconfig
SRCTREE = $(abspath ..)

# Parse .config file and set Make variables
ifneq ("$(wildcard $(OUTPUT_CONFIG))","")
# Read .config, skip comments and empty lines
CONFIG_LINES := $(shell grep -v '^#' $(OUTPUT_CONFIG) 2>/dev/null | grep -v '^$$')
# For each line, set variable
$(foreach line,$(CONFIG_LINES),\
	$(if $(findstring =,$(line)),\
		$(eval _VAR := $(word 1,$(subst =, ,$(line))))\
		$(eval _VAL := $(word 2,$(subst =, ,$(line))))\
		$(if $(filter CONFIG_%,$(_VAR)),\
			$(eval $(_VAR) := $(_VAL))\
			$(eval $(patsubst CONFIG_%,%,$(_VAR)) := $(_VAL))\
		)\
	)\
)
endif

# config target 
TARGET_NAME = osz
TARGET = $(TARGET_NAME).bin

# config qemu
QEMU := $(QEMU_PATH)/bin/qemu-system-riscv32
QFLAGS := -nographic -smp 1 -machine virt -bios none -m 128M

# Generate OSZ_CFG_MACRO from config variables
# First, collect all variables that start with OSZ_CFG_ or OSZ_
OSZ_CONFIG_VARS := $(filter OSZ_CFG_% OSZ_%,$(.VARIABLES))
OSZ_CFG_MACRO :=
$(foreach var,$(OSZ_CONFIG_VARS),\
	$(if $(filter y,$($(var))),\
		$(eval OSZ_CFG_MACRO += -D$(var)),\
		$(if $(filter-out y n,$($(var))),\
			$(eval OSZ_CFG_MACRO += -D$(var)=$($(var))),\
		)\
	)\
)

# config module, this is relative path which from G_PROJECT_PATH
MODULES := $(MODULES_y)

# config link global options
LDFLAGS := $(OSZ_UNDEFUINE_SYMB) -T$(OSZ_LD) -Map=$(TARGET_NAME).map -static
LDFLAGS += --gc-sections
LINK_PATH := $(foreach mod,$(MODULES),$(addprefix -L$(G_OUTPUT_PATH)/,$(basename $(notdir $(mod)))))
LINK_PATH := $(addsuffix /lib,$(LINK_PATH))
LIBS := --start-group
$(foreach mod,$(MODULES),$(eval $(call add_libs,$(basename $(notdir $(mod))))))
LIBS += --end-group

# config $(MAKE) options
MAKEFLAGS += --no-print-directory

# Kconfig targets
.PHONY: menuconfig oldconfig defconfig gen_header

menuconfig:
	@if command -v menuconfig >/dev/null 2>&1; then \
		KCONFIG_CONFIG=$(OUTPUT_CONFIG) srctree=$(SRCTREE) menuconfig $(KCONFIG_SRC); \
	else \
		echo "Error: menuconfig not found. Install with: pip install kconfiglib"; \
		exit 1; \
	fi

oldconfig:
	@if command -v menuconfig >/dev/null 2>&1; then \
		KCONFIG_CONFIG=$(OUTPUT_CONFIG) srctree=$(SRCTREE) menuconfig --oldconfig $(KCONFIG_SRC); \
	else \
		echo "Error: menuconfig not found. Install with: pip install kconfiglib"; \
		exit 1; \
	fi

defconfig:
	@echo "Generating default configuration from Kconfig..."
	@python3 $(SCRIPT_DIR)/gen_defconfig.py $(KCONFIG_SRC) $(OUTPUT_CONFIG)

gen_header:
	@if [ -f $(OUTPUT_CONFIG) ]; then \
		echo "Generating header from $(OUTPUT_CONFIG)..."; \
		python3 $(SCRIPT_DIR)/parser_cfg.py; \
	else \
		echo "Error: $(OUTPUT_CONFIG) not found. Run 'make defconfig' first."; \
		exit 1; \
	fi

# Default target requires .config
.config:
	@echo "Error: No .config file found."
	@echo "Please run one of the following commands first:"
	@echo "  make menuconfig   - Interactive configuration"
	@echo "  make defconfig    - Generate default configuration"
	@echo "  make oldconfig    - Update existing configuration"
	@exit 1

all: gen_header $(TARGET)
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/image
	$(HIDE)mv $(TARGET_NAME).asm $(TARGET_NAME).bin $(TARGET_NAME).elf $(TARGET_NAME).map $(G_OUTPUT_PATH)/image
	$(HIDE)echo "====== Build $^ Successful ======"

$(TARGET):
	$(foreach mod,$(MODULES),$(call build_module_target,$(mod)))
	$(HIDE)$(LD) $(LDFLAGS) $(LINK_PATH) $(LIBS) -o $(TARGET_NAME).elf
	$(HIDE)$(OBJCOPY) -O binary -S $(TARGET_NAME).elf $@
	$(HIDE)$(OBJDUMP) -S -d $(TARGET_NAME).elf > $(TARGET_NAME).asm

debug: all
	$(HIDE)echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	$(HIDE)echo "Press Ctrl-A-X to quit QEMU"
	$(HIDE)echo "-------------------------------------------------------"
	$(HIDE)$(G_BUILD_PATH)/script/kill_qemu.sh
	$(HIDE)${QEMU} ${QFLAGS} -kernel $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -s -S &
	$(HIDE)${GDB} $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -q -x $(G_BUILD_PATH)/script/gdbinit

debug_serv: all
	$(HIDE)echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	$(HIDE)echo "Press Ctrl-A-X to quit QEMU"
	$(HIDE)echo "-------------------------------------------------------"
	$(HIDE)$(G_BUILD_PATH)/script/kill_qemu.sh
	$(HIDE)${QEMU} ${QFLAGS} -kernel $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -s -S

debug_client:
	$(HIDE)${GDB} $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -q -x $(G_BUILD_PATH)/script/gdbinit

run: all
	$(HIDE)${QEMU} -M ? | grep virt >/dev/null || exit
	$(HIDE)echo "Press Ctrl-A and then X to exit QEMU"
	$(HIDE)echo "------------------------------------"
	$(HIDE)${QEMU} ${QFLAGS} -kernel $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf

clean:
	$(HIDE)rm -rf *.elf *.bin *.asm *.map
	$(HIDE)rm -rf output
