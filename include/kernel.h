#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define KERNEL_VERSION "0.1.0"
#define KERNEL_NAME "Vaelen Linux"
#define KERNEL_CODENAME "Genesis"

#define PAGE_SIZE 4096
#define KERNEL_BASE 0xC0000000
#define KERNEL_HEAP_START 0xD0000000
#define KERNEL_HEAP_SIZE (16 * 1024 * 1024)

void kernel_main(uint32_t magic, uint32_t addr);
void kernel_panic(const char *msg);

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

void cli(void);
void sti(void);

#endif