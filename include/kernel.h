#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

void kernel_main(uint32_t magic, uint32_t addr);
void printk(const char *str);
void printk_hex(uint32_t value);

#endif