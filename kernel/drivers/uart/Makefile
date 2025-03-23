include $(COMM_MK)

MODULE_NAME := uart
MODULE := libuart.a

C_SRCS = $(wildcard src/hal/*.c src/ext/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/trap/interrupt/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/hal
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -Wno-misleading-indentation

include $(MODULE_MK)