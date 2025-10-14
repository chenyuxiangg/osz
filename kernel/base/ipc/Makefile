include $(COMM_MK)

MODULE_NAME := ipc
MODULE := libipc.a

C_SRCS := $(wildcard src/event/*.c)
C_SRCS += $(wildcard src/sem/*.c)

OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)