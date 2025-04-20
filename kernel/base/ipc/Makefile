include $(COMM_MK)

MODULE_NAME := ipc
MODULE := libipc.a

C_SRCS := $(wildcard src/atomic/*.c)
C_SRCS += $(wildcard src/event/*.c)

OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)