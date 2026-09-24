CROSS_COMPILE = xtensa-esp32s3-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

CFLAGS = -std=c11 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs \
         -Wall -Wextra -Werror -O2 -g \
         -mlongcalls -mtext-section-literals \
         -Iinclude -Iarch/xtensa/include \
         -DBAREOS_VERSION=\"0.1.0\" \
         -DBAREOS_ARCH=\"xtensa\"

ASFLAGS = -mlongcalls -mtext-section-literals \
          -Iarch/xtensa/include

LDFLAGS = -T linker/bareos.ld -nostdlib -nostartfiles -nodefaultlibs \
          -Wl,--gc-sections -Wl,-Map=build/bareos.map

KERNEL_SRCS = \
    arch/xtensa/boot/startup.S \
    arch/xtensa/cpu/cpu.c \
    arch/xtensa/context/context.S \
    arch/xtensa/exceptions/vectors.S \
    arch/xtensa/traps/traps.c \
    arch/xtensa/timer/timer.c \
    arch/xtensa/atomic/atomic.c \
    kernel/core/kernel.c \
    kernel/scheduler/scheduler.c \
    kernel/task/task.c \
    kernel/process/process.c \
    kernel/syscall/syscall.c \
    kernel/memory/memory.c \
    kernel/ipc/ipc.c \
    kernel/sync/sync.c \
    kernel/signal/signal.c \
    kernel/time/time.c \
    kernel/panic/panic.c \
    drivers/uart/uart.c \
    drivers/gpio/gpio.c \
    drivers/timer/timer.c \
    drivers/flash/flash.c \
    drivers/psram/psram.c \
    drivers/spi/spi.c \
    drivers/i2c/i2c.c \
    drivers/dma/dma.c \
    drivers/console/console.c \
    fs/vfs/vfs.c \
    fs/vfs/vfs_ops.c \
    fs/ramfs/ramfs.c \
    fs/devfs/devfs.c \
    fs/storage/storage.c \
    libc/string/string.c \
    libc/stdio/stdio.c \
    libc/stdlib/stdlib.c \
    libc/ctype/ctype.c \
    libc/time/time.c \
    libc/posix/posix.c

USERLAND_SRCS = \
    userland/bin/ls.c \
    userland/bin/cat.c \
    userland/bin/echo.c \
    userland/bin/ps.c \
    userland/bin/pwd.c \
    userland/bin/env.c \
    userland/bin/sleep.c \
    userland/bin/uname.c \
    userland/bin/clear.c \
    userland/bin/mkdir.c \
    userland/bin/rmdir.c \
    userland/bin/touch.c \
    userland/bin/kill.c \
    userland/bin/date.c \
    userland/bin/uptime.c \
    userland/bin/head.c \
    userland/bin/tail.c \
    userland/bin/wc.c \
    userland/bin/id.c \
    userland/bin/whoami.c \
    userland/bin/true.c \
    userland/bin/false.c \
    userland/bin/test.c \
    userland/sbin/reboot.c \
    userland/sbin/halt.c \
    userland/sbin/poweroff.c \
    userland/sh/shell.c

KERNEL_OBJS = $(patsubst %.c,build/%.o,$(filter %.c,$(KERNEL_SRCS)))
KERNEL_OBJS += $(patsubst %.S,build/%.o,$(filter %.S,$(KERNEL_SRCS)))

USERLAND_OBJS = $(patsubst %.c,build/%.o,$(USERLAND_SRCS))

all: build/bareos.elf build/bareos.bin build/bareos.map

build/bareos.elf: $(KERNEL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

build/bareos.bin: build/bareos.elf
	$(OBJCOPY) -O binary $< $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ASFLAGS) -c -o $@ $<

clean:
	rm -rf build

flash: build/bareos.bin
	esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 $<

monitor:
	python3 -m serial.tools.miniterm /dev/ttyUSB0 115200

erase:
	esptool.py --chip esp32s3 --port /dev/ttyUSB0 erase_flash

size: build/bareos.elf
	$(SIZE) $<

disassemble: build/bareos.elf
	$(OBJDUMP) -d $< > build/bareos.dis

.PHONY: all clean flash monitor erase size disassemble