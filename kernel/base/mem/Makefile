include $(COMM_MK)

MODULE_NAME := mem
MODULE := libmem.a

C_SRCS = $(wildcard bestfit/src/*.c)
C_SRCS += $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

LOCAL_FLAGS += -Wno-array-bounds -Wno-int-conversion

include $(MODULE_MK)