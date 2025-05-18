include $(COMM_MK)

MODULE_NAME := schedule
MODULE := libschedule.a

ifeq ($(OSZ_CFG_PREEMPTION), y)
C_SRCS := $(wildcard preemption/src/*.c)
C_SRCS += $(wildcard preemption/priority_queue/src/*.c)
C_SRCS += $(wildcard preemption/sortlink/src/*.c)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/preemption/priority_queue/include
endif
ifeq ($(OSZ_CFG_COORDINATION), y)
C_SRCS := $(wildcard coordination/src/*.c)
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
endif
S_SRCS := $(wildcard src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)