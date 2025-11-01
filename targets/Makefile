include $(COMM_MK)

MODULE_NAME := targets
MODULE := libtargets.a

ifeq ($(OSZ_CFG_TEST_SUITE), y)
C_SRCS = $(wildcard qemu_virt/src/test_main.c)
else
C_SRCS = $(wildcard qemu_virt/src/main.c)
endif
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-array-bounds

include $(MODULE_MK)