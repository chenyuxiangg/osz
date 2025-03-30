include $(COMM_MK)

MODULE_NAME := struct
MODULE := libstruct.a

C_SRCS = $(wildcard double_link/src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include

include $(MODULE_MK)