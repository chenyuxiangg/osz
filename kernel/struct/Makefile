include $(COMM_MK)

MODULE_NAME := struct
MODULE := libstruct.a

C_SRCS = $(wildcard double_link/src/*.c)
C_SRCS += $(wildcard fifo/src/*.c)
C_SRCS += $(wildcard sortlink/src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)