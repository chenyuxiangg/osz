include $(COMM_MK)

MODULE_NAME := trap
MODULE := libtrap.a

C_SRCS := $(wildcard *.c)
C_SRCS += $(wildcard exception/src/*.c)
C_SRCS += $(wildcard interrupt/src/*.c)
S_SRCS := $(wildcard src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)