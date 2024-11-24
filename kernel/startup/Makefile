include $(COMM_MK)

MODULE_NAME := startup
MODULE := libstartup.a

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include

include $(MODULE_MK)