include $(COMM_MK)

MODULE_NAME := struct
MODULE := libstruct.a

C_SRCS = $(wildcard double_link/src/*.c)
C_SRCS += $(wildcard priority_queue/src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/priority_queue/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include

include $(MODULE_MK)