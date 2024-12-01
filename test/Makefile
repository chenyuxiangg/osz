include $(COMM_MK)

MODULE_NAME := test
MODULE := libtest.a

C_SRCS = $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/ext
LOCAL_FLAGS += -I$(G_KERNEL_DRIVERS_PATH)/uart/include/hal
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/bestfit/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include
LOCAL_FLAGS += -I$(G_OPENSOURCE_MUSL_PATH)/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/task/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/schedule/coordination/include

LOCAL_FLAGS += -Wno-array-bounds

include $(MODULE_MK)