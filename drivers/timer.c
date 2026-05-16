#include "kernel.h"
#include "isr.h"
#include "task.h"
#include "vga.h"

static uint32_t timer_ticks = 0;
static uint32_t timer_frequency = 100;

static void timer_handler(registers_t *regs) {
    timer_ticks++;

    task_t *task = task_get_list_head();
    while (task) {
        if (task->state == TASK_SLEEPING && task->sleep_ticks > 0) {
            task->sleep_ticks--;
            if (task->sleep_ticks == 0) {
                task->state = TASK_READY;
            }
        }
        task = task->next;
    }
}

void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

    irq_install_handler(0, timer_handler);
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_sleep(uint32_t ms) {
    uint32_t target = timer_ticks + (ms * timer_frequency / 1000);
    while (timer_ticks < target) {
        asm volatile("hlt");
    }
}