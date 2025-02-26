include $(COMM_MK)

MODULE_NAME := task
MODULE := libtask.a

C_SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include

include $(MODULE_MK)