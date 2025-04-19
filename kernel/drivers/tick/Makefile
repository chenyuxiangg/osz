include $(COMM_MK)

MODULE_NAME := tick
MODULE := libtick.a

C_SRCS = $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-misleading-indentation

include $(MODULE_MK)