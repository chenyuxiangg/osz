include $(COMM_MK)

MODULE_NAME := init
MODULE := libinit.a

C_SRCS = $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/struct/double_link/include
LOCAL_FLAGS += -I$(G_KERNEL_COMM_PATH)/include
LOCAL_FLAGS += -I$(G_PLATFORM_PATH)/qemu_virt/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/bestfit/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/mem/include
LOCAL_FLAGS += -I$(G_KERNEL_BASE_PATH)/init/include
LOCAL_FLAGS += -I$(G_KERNEL_DEBUG_PATH)/include

LOCAL_FLAGS += -Wno-array-bounds -Wno-int-conversion

include $(MODULE_MK)