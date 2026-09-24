@echo off
set PATH=C:\xtensa-toolchain\xtensa-esp32s3-elf\bin;%PATH%

set CC=xtensa-esp32s3-elf-gcc
set AS=xtensa-esp32s3-elf-as
set LD=xtensa-esp32s3-elf-ld
set OBJCOPY=xtensa-esp32s3-elf-objcopy
set OBJDUMP=xtensa-esp32s3-elf-objdump
set SIZE=xtensa-esp32s3-elf-size

set CFLAGS=-std=c11 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -O2 -g -mlongcalls -mtext-section-literals -mno-target-align -Iinclude -Iarch/xtensa/include -Ilibc/include -Ikernel -DBAREOS_VERSION="0.1.0" -DBAREOS_ARCH="xtensa"
set ASFLAGS=-mlongcalls -mtext-section-literals -mno-target-align -Iarch/xtensa/include
set LDFLAGS=-T linker/bareos.ld -nostdlib -nostartfiles -nodefaultlibs -Wl,--gc-sections -Wl,-Map=build/bareos.map

if not exist build mkdir build
if not exist build\arch\xtensa\boot mkdir build\arch\xtensa\boot
if not exist build\arch\xtensa\cpu mkdir build\arch\xtensa\cpu
if not exist build\arch\xtensa\context mkdir build\arch\xtensa\context
if not exist build\arch\xtensa\exceptions mkdir build\arch\xtensa\exceptions
if not exist build\arch\xtensa\traps mkdir build\arch\xtensa\traps
if not exist build\arch\xtensa\timer mkdir build\arch\xtensa\timer
if not exist build\arch\xtensa\atomic mkdir build\arch\xtensa\atomic
if not exist build\kernel\core mkdir build\kernel\core
if not exist build\kernel\scheduler mkdir build\kernel\scheduler
if not exist build\kernel\task mkdir build\kernel\task
if not exist build\kernel\process mkdir build\kernel\process
if not exist build\kernel\syscall mkdir build\kernel\syscall
if not exist build\kernel\memory mkdir build\kernel\memory
if not exist build\kernel\exec mkdir build\kernel\exec
if not exist build\kernel\ipc mkdir build\kernel\ipc
if not exist build\kernel\sync mkdir build\kernel\sync
if not exist build\kernel\signal mkdir build\kernel\signal
if not exist build\kernel\time mkdir build\kernel\time
if not exist build\kernel\panic mkdir build\kernel\panic
if not exist build\drivers\uart mkdir build\drivers\uart
if not exist build\drivers\gpio mkdir build\drivers\gpio
if not exist build\drivers\timer mkdir build\drivers\timer
if not exist build\drivers\flash mkdir build\drivers\flash
if not exist build\drivers\psram mkdir build\drivers\psram
if not exist build\drivers\spi mkdir build\drivers\spi
if not exist build\drivers\i2c mkdir build\drivers\i2c
if not exist build\drivers\dma mkdir build\drivers\dma
if not exist build\drivers\console mkdir build\drivers\console
if not exist build\fs\vfs mkdir build\fs\vfs
if not exist build\fs\ramfs mkdir build\fs\ramfs
if not exist build\fs\devfs mkdir build\fs\devfs
if not exist build\fs\storage mkdir build\fs\storage
if not exist build\libc\string mkdir build\libc\string
if not exist build\libc\stdio mkdir build\libc\stdio
if not exist build\libc\stdlib mkdir build\libc\stdlib
if not exist build\libc\ctype mkdir build\libc\ctype
if not exist build\libc\time mkdir build\libc\time
if not exist build\libc\posix mkdir build\libc\posix
if not exist build\userland\bin mkdir build\userland\bin
if not exist build\userland\sbin mkdir build\userland\sbin
if not exist build\userland\usr.bin mkdir build\userland\usr.bin
if not exist build\userland\usr.sbin mkdir build\userland\usr.sbin
if not exist build\userland\sh mkdir build\userland\sh
if not exist build\userland\init mkdir build\userland\init

echo Compiling arch/xtensa/boot/startup.S
%CC% %CFLAGS% %ASFLAGS% -c arch/xtensa/boot/startup.S -o build/arch/xtensa/boot/startup.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/cpu/cpu.c
%CC% %CFLAGS% -c arch/xtensa/cpu/cpu.c -o build/arch/xtensa/cpu/cpu.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/cpu/xtensa_regs.S
%CC% %CFLAGS% %ASFLAGS% -c arch/xtensa/cpu/xtensa_regs.S -o build/arch/xtensa/cpu/xtensa_regs.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/context/context.S
%CC% %CFLAGS% %ASFLAGS% -c arch/xtensa/context/context.S -o build/arch/xtensa/context/context.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/exceptions/vectors.S
%CC% %CFLAGS% %ASFLAGS% -c arch/xtensa/exceptions/vectors.S -o build/arch/xtensa/exceptions/vectors.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/traps/traps.c
%CC% %CFLAGS% -c arch/xtensa/traps/traps.c -o build/arch/xtensa/traps/traps.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/timer/timer.c
%CC% %CFLAGS% -c arch/xtensa/timer/timer.c -o build/arch/xtensa/timer/timer.o
if errorlevel 1 exit /b 1

echo Compiling arch/xtensa/atomic/atomic.c
%CC% %CFLAGS% -c arch/xtensa/atomic/atomic.c -o build/arch/xtensa/atomic/atomic.o
if errorlevel 1 exit /b 1

echo Compiling kernel/core/kernel.c
%CC% %CFLAGS% -c kernel/core/kernel.c -o build/kernel/core/kernel.o
if errorlevel 1 exit /b 1

echo Compiling kernel/scheduler/scheduler.c
%CC% %CFLAGS% -c kernel/scheduler/scheduler.c -o build/kernel/scheduler/scheduler.o
if errorlevel 1 exit /b 1

echo Compiling kernel/task/task.c
%CC% %CFLAGS% -c kernel/task/task.c -o build/kernel/task/task.o
if errorlevel 1 exit /b 1

echo Compiling kernel/process/process.c
%CC% %CFLAGS% -c kernel/process/process.c -o build/kernel/process/process.o
if errorlevel 1 exit /b 1

echo Compiling kernel/syscall/syscall.c
%CC% %CFLAGS% -c kernel/syscall/syscall.c -o build/kernel/syscall/syscall.o
if errorlevel 1 exit /b 1

echo Compiling kernel/exec/exec.c
%CC% %CFLAGS% -c kernel/exec/exec.c -o build/kernel/exec/exec.o
if errorlevel 1 exit /b 1

echo Compiling kernel/memory/memory.c
%CC% %CFLAGS% -c kernel/memory/memory.c -o build/kernel/memory/memory.o
if errorlevel 1 exit /b 1

echo Compiling kernel/ipc/ipc.c
%CC% %CFLAGS% -c kernel/ipc/ipc.c -o build/kernel/ipc/ipc.o
if errorlevel 1 exit /b 1

echo Compiling kernel/sync/sync.c
%CC% %CFLAGS% -c kernel/sync/sync.c -o build/kernel/sync/sync.o
if errorlevel 1 exit /b 1

echo Compiling kernel/signal/signal.c
%CC% %CFLAGS% -c kernel/signal/signal.c -o build/kernel/signal/signal.o
if errorlevel 1 exit /b 1

echo Compiling kernel/time/time.c
%CC% %CFLAGS% -c kernel/time/time.c -o build/kernel/time/time.o
if errorlevel 1 exit /b 1

echo Compiling kernel/panic/panic.c
%CC% %CFLAGS% -c kernel/panic/panic.c -o build/kernel/panic/panic.o
if errorlevel 1 exit /b 1

echo Compiling drivers/uart/uart.c
%CC% %CFLAGS% -c drivers/uart/uart.c -o build/drivers/uart/uart.o
if errorlevel 1 exit /b 1

echo Compiling drivers/gpio/gpio.c
%CC% %CFLAGS% -c drivers/gpio/gpio.c -o build/drivers/gpio/gpio.o
if errorlevel 1 exit /b 1

echo Compiling drivers/timer/timer.c
%CC% %CFLAGS% -c drivers/timer/timer.c -o build/drivers/timer/timer.o
if errorlevel 1 exit /b 1

echo Compiling drivers/flash/flash.c
%CC% %CFLAGS% -c drivers/flash/flash.c -o build/drivers/flash/flash.o
if errorlevel 1 exit /b 1

echo Compiling drivers/psram/psram.c
%CC% %CFLAGS% -c drivers/psram/psram.c -o build/drivers/psram/psram.o
if errorlevel 1 exit /b 1

echo Compiling drivers/spi/spi.c
%CC% %CFLAGS% -c drivers/spi/spi.c -o build/drivers/spi/spi.o
if errorlevel 1 exit /b 1

echo Compiling drivers/i2c/i2c.c
%CC% %CFLAGS% -c drivers/i2c/i2c.c -o build/drivers/i2c/i2c.o
if errorlevel 1 exit /b 1

echo Compiling drivers/dma/dma.c
%CC% %CFLAGS% -c drivers/dma/dma.c -o build/drivers/dma/dma.o
if errorlevel 1 exit /b 1

echo Compiling drivers/console/console.c
%CC% %CFLAGS% -c drivers/console/console.c -o build/drivers/console/console.o
if errorlevel 1 exit /b 1

echo Compiling fs/vfs/vfs.c
%CC% %CFLAGS% -c fs/vfs/vfs.c -o build/fs/vfs/vfs.o
if errorlevel 1 exit /b 1

echo Compiling fs/ramfs/ramfs.c
%CC% %CFLAGS% -c fs/ramfs/ramfs.c -o build/fs/ramfs/ramfs.o
if errorlevel 1 exit /b 1

echo Compiling fs/devfs/devfs.c
%CC% %CFLAGS% -c fs/devfs/devfs.c -o build/fs/devfs/devfs.o
if errorlevel 1 exit /b 1

echo Compiling fs/storage/storage.c
%CC% %CFLAGS% -c fs/storage/storage.c -o build/fs/storage/storage.o
if errorlevel 1 exit /b 1

echo Compiling libc/string/string.c
%CC% %CFLAGS% -c libc/string/string.c -o build/libc/string/string.o
if errorlevel 1 exit /b 1

echo Compiling libc/stdio/stdio.c
%CC% %CFLAGS% -c libc/stdio/stdio.c -o build/libc/stdio/stdio.o
if errorlevel 1 exit /b 1

echo Compiling libc/stdlib/stdlib.c
%CC% %CFLAGS% -c libc/stdlib/stdlib.c -o build/libc/stdlib/stdlib.o
if errorlevel 1 exit /b 1

echo Compiling libc/ctype/ctype.c
%CC% %CFLAGS% -c libc/ctype/ctype.c -o build/libc/ctype/ctype.o
if errorlevel 1 exit /b 1

echo Compiling libc/time/time.c
%CC% %CFLAGS% -c libc/time/time.c -o build/libc/time/time.o
if errorlevel 1 exit /b 1

echo Compiling libc/posix/posix.c
%CC% %CFLAGS% -c libc/posix/posix.c -o build/libc/posix/posix.o
if errorlevel 1 exit /b 1

echo Compiling userland/init/init.c
%CC% %CFLAGS% -c userland/init/init.c -o build/userland/init/init.o
if errorlevel 1 exit /b 1

echo Compiling userland/sh/shell.c
%CC% %CFLAGS% -c userland/sh/shell.c -o build/userland/sh/shell.o
if errorlevel 1 exit /b 1

echo Linking build/bareos.elf
%CC% %LDFLAGS% -o build/bareos.elf ^
  build/arch/xtensa/boot/startup.o ^
  build/arch/xtensa/cpu/cpu.o ^
  build/arch/xtensa/cpu/xtensa_regs.o ^
  build/arch/xtensa/context/context.o ^
  build/arch/xtensa/exceptions/vectors.o ^
  build/arch/xtensa/traps/traps.o ^
  build/arch/xtensa/timer/timer.o ^
  build/arch/xtensa/atomic/atomic.o ^
  build/kernel/core/kernel.o ^
  build/kernel/scheduler/scheduler.o ^
  build/kernel/task/task.o ^
  build/kernel/process/process.o ^
  build/kernel/syscall/syscall.o ^
  build/kernel/exec/exec.o ^
  build/kernel/memory/memory.o ^
  build/kernel/ipc/ipc.o ^
  build/kernel/sync/sync.o ^
  build/kernel/signal/signal.o ^
  build/kernel/time/time.o ^
  build/kernel/panic/panic.o ^
  build/drivers/uart/uart.o ^
  build/drivers/gpio/gpio.o ^
  build/drivers/timer/timer.o ^
  build/drivers/flash/flash.o ^
  build/drivers/psram/psram.o ^
  build/drivers/spi/spi.o ^
  build/drivers/i2c/i2c.o ^
  build/drivers/dma/dma.o ^
  build/drivers/console/console.o ^
  build/fs/vfs/vfs.o ^
  build/fs/ramfs/ramfs.o ^
  build/fs/devfs/devfs.o ^
  build/fs/storage/storage.o ^
  build/libc/string/string.o ^
  build/libc/stdio/stdio.o ^
  build/libc/stdlib/stdlib.o ^
  build/libc/ctype/ctype.o ^
  build/libc/time/time.o ^
  build/libc/posix/posix.o ^
  build/userland/init/init.o ^
  build/userland/sh/shell.o
if errorlevel 1 exit /b 1

echo Creating build/bareos.bin
%OBJCOPY% -O binary build/bareos.elf build/bareos.bin
if errorlevel 1 exit /b 1

echo Build successful!
%SIZE% build/bareos.elf