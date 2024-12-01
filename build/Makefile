include ./make/comm.mk
include ./config/zos.config

# config target 
TARGET_NAME = zos
TARGET = $(TARGET_NAME).bin

# config qemu
QEMU := $(QEMU_PATH)/bin/qemu-system-riscv32
QFLAGS := -nographic -smp 1 -machine virt -bios none -m 128M

# config module, this is relative path which from G_PROJECT_PATH
MODULES_y :=
MODULES_n :=
MODULES_$(ZOS_CFG_STARTUP) += kernel/startup
MODULES_$(ZOS_CFG_STRUCT) += kernel/base/struct
MODULES_$(ZOS_CFG_COMM) += kernel/comm
MODULES_$(ZOS_CFG_MEM) += kernel/base/mem
MODULES_$(ZOS_CFG_UART) += kernel/drivers/uart
MODULES_$(ZOS_CFG_DEBUG) += kernel/debug
MODULES_$(ZOS_CFG_SCHEDULE) += kernel/base/schedule
MODULES_$(ZOS_CFG_TASK) += kernel/base/task
MODULES_$(ZOS_CFG_MUSL) += opensource/musl
MODULES_$(ZOS_CFG_TEST) += test

MODULES := $(MODULES_y)

# config link global options
LDFLAGS := -T$(ZOS_LD) -Map=$(TARGET_NAME).map --gc-sections -static
LINK_PATH := $(foreach mod,$(MODULES),$(addprefix -L$(G_OUTPUT_PATH)/,$(basename $(notdir $(mod)))))
LINK_PATH := $(addsuffix /lib,$(LINK_PATH))
LIBS := --start-group
$(foreach mod,$(MODULES),$(eval $(call add_libs,$(basename $(notdir $(mod))))))
LIBS += --end-group

# config $(MAKE) options
MAKEFLAGS += --no-print-directory

all: $(TARGET)
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/image
	$(HIDE)mv $(TARGET_NAME).asm $(TARGET_NAME).bin $(TARGET_NAME).elf $(TARGET_NAME).map $(G_OUTPUT_PATH)/image
	$(HIDE)echo "====== Build $^ Successful ======"

$(TARGET):
	$(foreach mod,$(MODULES),$(call build_module_target,$(mod)))
	$(HIDE)$(LD) $(LDFLAGS) $(LINK_PATH) $(LIBS) -o $(TARGET_NAME).elf
	$(HIDE)$(OBJCOPY) -O binary -S $(TARGET_NAME).elf $@
	$(HIDE)$(OBJDUMP) -d $(TARGET_NAME).elf > $(TARGET_NAME).asm

debug: all
	$(HIDE)echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	$(HIDE)echo "Press Ctrl-A-X to quit QEMU"
	$(HIDE)echo "-------------------------------------------------------"
	$(HIDE)$(G_BUILD_PATH)/script/kill_qemu.sh
	$(HIDE)${QEMU} ${QFLAGS} -kernel $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -s -S &
	$(HIDE)${GDB} $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf -q -x $(G_BUILD_PATH)/script/gdbinit

run: all
	$(HIDE)${QEMU} -M ? | grep virt >/dev/null || exit
	$(HIDE)echo "Press Ctrl-A and then X to exit QEMU"
	$(HIDE)echo "------------------------------------"
	$(HIDE)${QEMU} ${QFLAGS} -kernel $(G_OUTPUT_PATH)/image/$(TARGET_NAME).elf

clean:
	$(HIDE)rm -rf *.elf *.bin *.asm *.map
	$(HIDE)rm -rf output