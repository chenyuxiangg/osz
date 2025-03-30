include $(COMM_MK)

MODULE_NAME := task
MODULE := libtask.a

C_SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/bestfit/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
ifeq ($(OSZ_CFG_SCHEDULE_PREEMPTION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/priority_queue/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/sortlink/include
endif
ifeq ($(OSZ_CFG_SCHEDULE_COORDINATION), y)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
endif
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/timer/include/hal

include $(MODULE_MK)