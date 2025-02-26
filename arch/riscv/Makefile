include $(COMM_MK)

MODULE_NAME := riscv
MODULE := libriscv.a

C_SRCS := $(wildcard src/*.c)
S_SRCS := $(wildcard src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include

include $(MODULE_MK)