include $(COMM_MK)

MODULE_NAME := ipc
MODULE := libipc.a
C_SRCS :=
ifeq ($(OSZ_CFG_EVENT), y)
C_SRCS += $(wildcard src/event/*.c)
endif
ifeq ($(OSZ_CFG_SEM), y)
C_SRCS += $(wildcard src/sem/*.c)
endif
ifeq ($(OSZ_CFG_MUTEX), y)
C_SRCS += $(wildcard src/mutex/*.c)
endif

OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)