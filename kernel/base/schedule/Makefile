include $(COMM_MK)

MODULE_NAME := schedule
MODULE := libschedule.a

C_SRCS := $(wildcard coordination/src/*.c)
S_SRCS := $(wildcard coordination/src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include

include $(MODULE_MK)