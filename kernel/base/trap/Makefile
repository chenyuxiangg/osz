include $(COMM_MK)

MODULE_NAME := trap
MODULE := libtrap.a

C_SRCS := $(wildcard *.c)
C_SRCS += $(wildcard exception/src/*.c)
C_SRCS += $(wildcard interrupt/src/*.c)
S_SRCS := $(wildcard src/*.S)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))
OBJS += $(patsubst %.S, %.o, $(S_SRCS))

LOCAL_FLAGS := -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_OPENSOURCE_MUSL_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/exception/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include/clint
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include/plic
LOCAL_FLAGS += -I$(G_ARCH_PATH)/riscv/include
LOCAL_FLAGS += -I$(G_PLATFORM_PATH)/qemu_virt/include


include $(MODULE_MK)