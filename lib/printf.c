#include <stdint.h>
#include "string.h"
#include "vga.h"

static void printf_putchar(char c, void *ctx) {
    vga_putchar(c);
}

static void printf_number(unsigned int num, int base, int width, int pad_zero) {
    char buf[32];
    itoa(num, buf, base);
    int len = strlen(buf);
    for (int i = len; i < width; i++) {
        vga_putchar(pad_zero ? '0' : ' ');
    }
    vga_puts(buf);
}

static void printf_signed(int num, int width, int pad_zero) {
    if (num < 0) {
        vga_putchar('-');
        printf_number(-num, 10, width - 1, pad_zero);
    } else {
        printf_number(num, 10, width, pad_zero);
    }
}

void printf(const char *fmt, ...) {
    int *args = (int *)&fmt;
    args++;

    while (*fmt) {
        if (*fmt != '%') {
            vga_putchar(*fmt++);
            continue;
        }
        fmt++;

        int width = 0;
        int pad_zero = 0;
        if (*fmt == '0') { pad_zero = 1; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
            case 'd':
            case 'i':
                printf_signed(*args++, width, pad_zero);
                break;
            case 'u':
                printf_number(*args++, 10, width, pad_zero);
                break;
            case 'x':
                printf_number(*args++, 16, width, pad_zero);
                break;
            case 'X':
                printf_number(*args++, 16, width, pad_zero);
                break;
            case 'o':
                printf_number(*args++, 8, width, pad_zero);
                break;
            case 'p':
                vga_puts("0x");
                printf_number(*args++, 16, 8, 1);
                break;
            case 's': {
                char *str = (char *)*args++;
                if (!str) str = "(null)";
                int len = strlen(str);
                for (int i = len; i < width; i++) vga_putchar(' ');
                vga_puts(str);
                break;
            }
            case 'c':
                vga_putchar((char)*args++);
                break;
            case '%':
                vga_putchar('%');
                break;
            default:
                vga_putchar('%');
                vga_putchar(*fmt);
                break;
        }
        fmt++;
    }
}