include $(COMM_MK)

MODULE_NAME := schedule
MODULE := libschedule.a

ifeq ($(OSZ_CFG_SCHEDULE_PREEMPTION), y)
C_SRCS := $(wildcard preemption/src/*.c)
C_SRCS += $(wildcard preemption/priority_queue/src/*.c)
C_SRCS += $(wildcard preemption/sortlink/src/*.c)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/priority_queue/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/sortlink/include
endif
ifeq ($(OSZ_CFG_SCHEDULE_COORDINATION), y)
C_SRCS := $(wildcard coordination/src/*.c)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
endif
S_SRCS := $(wildcard src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include

include $(MODULE_MK)