# config gcc compiler global options
CFLAGS := -nostdlib -fno-builtin -nostartfiles -nostdinc -ffunction-sections -fdata-sections -ggdb -Wall
CFLAGS += -march=rv32g -mabi=ilp32d
CFLAGS += -fPIC -static

all: clean $(MODULE)

# =====================================================
# Description:
#   判断一下OBJS是否为空再决定是否调用AR生成.a，因为部分第三方库
#   直接提供了.a，不需要重新编译。
# =====================================================
$(MODULE):$(OBJS)
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/$(MODULE_NAME)/obj
ifneq ($(OBJS),)
	$(HIDE)$(AR) cr $@ $(OBJS)
	$(HIDE)mv $(MODULE) $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib
	$(HIDE)mv $(OBJS) $(G_OUTPUT_PATH)/$(MODULE_NAME)/obj
else
	$(HIDE)cp lib/*.a $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib
	$(HIDE)cp lib/*.o $(G_OUTPUT_PATH)/$(MODULE_NAME)/obj
	$(HIDE)cd $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib && \
	for lib in $(MODULE_LIST); do \
	$(AR) -x $$lib; \
	done && \
	rm -rf *.a; \
	$(AR) cr lib$(MODULE).a *.o; \
	rm -rf *.o
endif

%.o:%.c
	$(HIDE)$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c -o $@ $<

%.o:%.S
	$(HIDE)$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c -o $@ $<

clean:
	$(HIDE)rm -rf *.a *.o