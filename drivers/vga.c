#include "kernel.h"
#include "vga.h"

static uint8_t vga_color;
static uint16_t *vga_buffer;
static uint8_t cursor_x;
static uint8_t cursor_y;

void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    vga_color = 0x07;
    vga_buffer = (uint16_t *)VGA_MEMORY;
    vga_clear();
}

void vga_setcolor(uint8_t color) {
    vga_color = color;
}

void vga_scroll(void) {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)vga_color << 8;
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)' ' | (uint16_t)vga_color << 8;
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)c | (uint16_t)vga_color << 8;
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
    vga_move_cursor(cursor_x, cursor_y);
}

void vga_write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_puts(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)vga_color << 8;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_move_cursor(0, 0);
}

void vga_move_cursor(uint8_t x, uint8_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 14);
    outb(0x3D5, (pos >> 8) & 0xFF);
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}

void vga_get_cursor(uint8_t *x, uint8_t *y) {
    *x = cursor_x;
    *y = cursor_y;
}