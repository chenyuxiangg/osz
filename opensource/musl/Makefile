# =====================================================
# musl是第三方库，因此：
#   1. C_SRCS 留空
#   2. MODULE 值与 MODULE_NAME相等
#	3. LOCAL_FLAGS 留空
# =====================================================
include $(COMM_MK)

MODULE_NAME := musl
MODULE := $(MODULE_NAME)
MODULE_LIST := libc.a libcrypt.a libdl.a libm.a libpthread.a libresolv.a librt.a libutil.a libxnet.a

C_SRCS :=
OBJS := $(patsubst %.c, %.o, $(C_SRCS))

LOCAL_FLAGS := 

include $(MODULE_MK)