include $(COMM_MK)

MODULE_NAME := comm
MODULE := libcomm.a

S_SRCS = $(wildcard src/*.S)
OBJS := $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include

include $(MODULE_MK)