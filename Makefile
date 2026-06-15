CC := clang-18
BIN := kexp.bin
SRC_DIR := src
BUILD_DIR := build

KEXP_NO_PTHREADS ?= 0

TARGET := -target x86_64-linux-gnu --sysroot=/usr/x86_64-linux-gnu

CFLAGS := $(TARGET) -O3 -Iinclude \
		-fPIE -fcommon -fno-omit-frame-pointer -fno-zero-initialized-in-bss \
		-ffreestanding -nostdlib -nostartfiles \
	    -Wall -Wextra -Werror -Wno-int-conversion -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-uninitialized \
        -DKEXP_NO_PTHREADS=$(KEXP_NO_PTHREADS)

LDFLAGS := $(TARGET) -fuse-ld=lld -nostdlib -nostartfiles -static -Wl,--build-id=none

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS)) $(BUILD_DIR)/syscalls.o

.PHONY: all clean

all: $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@
	
OBJCOPY := llvm-objcopy

$(BUILD_DIR)/$(BIN): $(OBJS) script.ld
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/kexp.elf -Tscript.ld
	$(OBJCOPY) -O binary $(BUILD_DIR)/kexp.elf $@

clean:
	rm -rf $(BUILD_DIR)
