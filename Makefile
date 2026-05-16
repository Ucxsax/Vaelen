CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-ld

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

SRCS = boot/boot.s kernel/kernel.c kernel/shell.c lib/string.c
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

TARGET = vaelen.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $< -o $@

iso: $(TARGET)
	mkdir -p iso/boot/grub
	cp $(TARGET) iso/boot/
	echo 'set timeout=5' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "Vaelen Linux" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot /boot/$(TARGET)' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o Vaelen-linux-0.1-TEST.iso iso

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf iso

.PHONY: all clean iso