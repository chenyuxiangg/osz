include $(COMM_MK)

MODULE_NAME := test
MODULE := libtest.a

C_SRCS = $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-array-bounds

include $(MODULE_MK)