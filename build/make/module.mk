# config gcc compiler global options
CFLAGS := -nostdlib -fno-builtin -nostartfiles -nostdinc -ffunction-sections -fdata-sections -ggdb -Wall
CFLAGS += -march=rv32g -mabi=ilp32d
CFLAGS += -fPIC -static

all: clean $(MODULE)

$(MODULE):$(OBJS)
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib
	$(HIDE)mkdir -p $(G_OUTPUT_PATH)/$(MODULE_NAME)/obj
	$(HIDE)$(AR) cr $@ $(OBJS)
	$(HIDE)mv $(MODULE) $(G_OUTPUT_PATH)/$(MODULE_NAME)/lib
	$(HIDE)mv $(OBJS) $(G_OUTPUT_PATH)/$(MODULE_NAME)/obj

%.o:%.c
	$(HIDE)$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c -o $@ $<

%.o:%.S
	$(HIDE)$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c -o $@ $<

clean:
	$(HIDE)rm -rf *.a *.o