include $(COMM_MK)

MODULE_NAME := targets
MODULE := libtargets.a

C_SRCS = $(wildcard qemu_virt/src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/hal
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/priority_queue/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/init/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/bestfit/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
ifeq ($(OSZ_CFG_SCHEDULE_PREEMPTION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/include
endif
ifeq ($(OSZ_CFG_SCHEDULE_COORDINATION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
endif
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/hal

LOCAL_FLAGS += -Wno-array-bounds

include $(MODULE_MK)