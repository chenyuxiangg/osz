include $(COMM_MK)

MODULE_NAME := targets
MODULE := libtargets.a

C_SRCS = $(wildcard qemu_virt/src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-array-bounds

include $(MODULE_MK)