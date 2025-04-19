include $(COMM_MK)

MODULE_NAME := timer
MODULE := libtimer.a

C_SRCS = $(wildcard src/hal/*.c src/ext/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-misleading-indentation

include $(MODULE_MK)