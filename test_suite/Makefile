include $(COMM_MK)

MODULE_NAME := test_suite
MODULE := libtest_suite.a

C_SRCS :=
ifeq ($(OSZ_CFG_TEST_SUITE), y)
C_SRCS += $(wildcard et_base/src/*.c)
C_SRCS += $(wildcard et_adapt/src/*.c)
endif
ifeq ($(OSZ_CFG_EVENT), y)
C_SRCS += $(wildcard suite/src/event/ut/*.c)
endif
ifeq ($(OSZ_CFG_SEM), y)
C_SRCS += $(wildcard suite/src/sem/ut/*.c)
C_SRCS += $(wildcard suite/src/sem/st/*.c)
endif
ifeq ($(OSZ_CFG_MUTEX), y)
C_SRCS += $(wildcard suite/src/mutex/ut/*.c)
endif
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 
LOCAL_FLAGS += -Wno-array-bounds

LOCAL_INCLUDE := -Iet_base/include
LOCAL_INCLUDE += -Isuite/include/st
LOCAL_FLAGS += $(LOCAL_INCLUDE)

include $(MODULE_MK)