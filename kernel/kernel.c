#include <stdint.h>
#include "kernel.h"
#include "vga.h"
#include "shell.h"
#include "mm.h"
#include "task.h"
#include "fs.h"
#include "stdio.h"

extern void gdt_init(void);
extern void idt_init(void);
extern void isr_init(void);
extern void irq_init(void);
extern void keyboard_init(void);
extern void timer_init(uint32_t freq);
extern void ata_init(void);
extern void syscall_init(void);

void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void kernel_main(uint32_t magic, uint32_t addr) {
    vga_init();
    vga_setcolor(0x07);

    printf("Booting %s v%s (%s)...\n", KERNEL_NAME, KERNEL_VERSION, KERNEL_CODENAME);

    gdt_init();
    printf("[OK] GDT initialized\n");

    idt_init();
    printf("[OK] IDT initialized\n");

    isr_init();
    printf("[OK] ISR handlers ready\n");

    irq_init();
    printf("[OK] IRQ remapped\n");

    timer_init(100);
    printf("[OK] PIT timer at 100Hz\n");

    keyboard_init();
    printf("[OK] PS/2 keyboard driver loaded\n");

    ata_init();
    printf("[OK] ATA disk driver initialized\n");

    syscall_init();
    printf("[OK] System call interface ready\n");

    uint32_t mem_size = 8 * 1024 * 1024 * 1024ULL;
    mm_init(mem_size, (uint32_t *)0x100000);
    printf("[OK] Memory manager: %d MB total\n", mem_size / 1024 / 1024);

    mm_init_paging();
    printf("[OK] Paging enabled\n");

    task_init();
    printf("[OK] Task scheduler initialized\n");

    fs_init();
    printf("[OK] Virtual filesystem mounted\n");

    printf("\n%s v%s (%s)\n", KERNEL_NAME, KERNEL_VERSION, KERNEL_CODENAME);
    printf("Kernel loaded successfully.\n\n");

    asm volatile("sti");

    shell_run();

    for (;;) {
        asm volatile("hlt");
    }
}