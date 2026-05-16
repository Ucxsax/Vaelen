#include <stdint.h>
#include "kernel.h"
#include "vga.h"
#include "shell.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    unsigned char uc = c;
    if (uc == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void printk(const char *str) {
    const char *p = str;
    while (*p) {
        terminal_putchar(*p++);
    }
}

void printk_hex(uint32_t value) {
    char hex[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buf[2 + (7 - i)] = hex[(value >> (4 * i)) & 0xF];
    }
    buf[10] = '\0';
    printk(buf);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    terminal_initialize();
    printk("Welcome to Vaelen Linux Kernel!\n");
    printk("Version 0.1\n");
    printk("Copyright (C) 2026 Vaelen Project\n\n");
    
    shell_run();
    
    while (1) {
        __asm__ __volatile__("hlt");
    }
}