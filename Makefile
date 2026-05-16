# Vaelen Linux Kernel - 编译脚本
# 在Docker中编译：docker build -t vaelen-build . && docker run --rm -v ${PWD}:/workspace vaelen-build
# 本地编译：需要 gcc-multilib, nasm, xorriso, grub-pc-bin

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -nostdinc -fno-stack-protector -Iinclude
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

KERNEL_SRCS = kernel/kernel.c kernel/shell.c kernel/mm.c kernel/task.c kernel/panic.c
ARCH_SRCS = arch/x86_64/boot.s arch/x86_64/gdt.c arch/x86_64/gdt_flush.s arch/x86_64/idt.c arch/x86_64/isr.c arch/x86_64/irq.c arch/x86_64/irq.s arch/x86_64/syscall.c
FS_SRCS = fs/vfs.c fs/ramfs.c
DRIVER_SRCS = drivers/keyboard.c drivers/timer.c drivers/ata.c drivers/vga.c
LIB_SRCS = lib/string.c lib/stdlib.c lib/printf.c

SRCS = $(KERNEL_SRCS) $(ARCH_SRCS) $(FS_SRCS) $(DRIVER_SRCS) $(LIB_SRCS)
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

TARGET = vaelen.bin
ISO_NAME = Vaelen-linux-0.1-TEST.iso
VERSION = 0.1.0

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

iso: $(TARGET)
	@mkdir -p iso/boot/grub
	@cp $(TARGET) iso/boot/
	@echo 'set timeout=5' > iso/boot/grub/grub.cfg
	@echo 'set default=0' >> iso/boot/grub/grub.cfg
	@echo 'menuentry "Vaelen Linux" {' >> iso/boot/grub/grub.cfg
	@echo '    multiboot /boot/$(TARGET)' >> iso/boot/grub/grub.cfg
	@echo '    boot' >> iso/boot/grub/grub.cfg
	@echo '}' >> iso/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO_NAME) iso 2>/dev/null || \
		xorriso -as mkisofs -R -b boot/grub/i386-pc/eltorito.img \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-o $(ISO_NAME) iso 2>/dev/null || \
		echo "WARNING: ISO creation requires grub-mkrescue or xorriso"

run: iso
	@qemu-system-i386 -cdrom $(ISO_NAME) -m 256M 2>/dev/null || \
		echo "WARNING: QEMU not found. Burn ISO to USB/CD to test on real hardware."

clean:
	@rm -f $(OBJS) $(TARGET)
	@rm -rf iso
	@rm -f $(ISO_NAME)

.PHONY: all iso run clean