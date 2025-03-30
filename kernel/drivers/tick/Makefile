include $(COMM_MK)

MODULE_NAME := tick
MODULE := libtick.a

C_SRCS = $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/hal
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/exception/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include/clint
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include/plic
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
ifeq ($(OSZ_CFG_SCHEDULE_PREEMPTION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/priority_queue/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/sortlink/include
endif
ifeq ($(OSZ_CFG_SCHEDULE_COORDINATION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
endif

LOCAL_FLAGS += -Wno-misleading-indentation

include $(MODULE_MK)