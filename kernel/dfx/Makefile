include $(COMM_MK)

MODULE_NAME := dfx
MODULE := libdfx.a

C_SRCS = $(wildcard src/task/*.c)
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS :=
LOCAL_FLAGS += -Wno-array-bounds -Wno-int-conversion

include $(MODULE_MK)