include $(COMM_MK)

MODULE_NAME := comm
MODULE := libcomm.a

S_SRCS = $(wildcard src/*.S)
OBJS := $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)