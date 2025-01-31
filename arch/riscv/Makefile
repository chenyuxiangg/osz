include $(COMM_MK)

MODULE_NAME := riscv
MODULE := libriscv.a

C_SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include

include $(MODULE_MK)