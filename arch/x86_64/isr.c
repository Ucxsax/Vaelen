#include "kernel.h"
#include "isr.h"
#include "vga.h"
#include "syscall.h"

isr_handler_t isr_handlers[256];
extern void isr128(void);

static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"
};

void isr_handler(registers_t *regs) {
    if (isr_handlers[regs->int_no] != 0) {
        isr_handler_t handler = isr_handlers[regs->int_no];
        handler(regs);
    }
}

void isr_register_handler(uint8_t n, isr_handler_t handler) {
    isr_handlers[n] = handler;
}

void isr_init(void) {
    memset(&isr_handlers, 0, sizeof(isr_handlers));
}

void irq_handler(registers_t *regs) {
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    if (isr_handlers[regs->int_no] != 0) {
        isr_handler_t handler = isr_handlers[regs->int_no];
        handler(regs);
    }
}

void irq_init(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void irq_install_handler(int irq, void (*handler)(registers_t *)) {
    isr_handlers[irq + 32] = handler;
}

void irq_uninstall_handler(int irq) {
    isr_handlers[irq + 32] = 0;
}