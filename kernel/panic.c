#include "kernel.h"
#include "vga.h"

void kernel_panic(const char *msg) {
    asm volatile("cli");
    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts("Kernel: ");
    vga_puts(KERNEL_NAME);
    vga_puts(" v");
    vga_puts(KERNEL_VERSION);
    vga_puts("\nError: ");
    vga_puts(msg);
    vga_puts("\n\nSystem halted. Please restart your computer.\n");
    for (;;) asm volatile("hlt");
}