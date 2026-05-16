#include "kernel.h"
#include "task.h"
#include "mm.h"
#include "string.h"
#include "vga.h"

static task_t *task_list_head = 0;
static task_t *current_task = 0;
static task_t *idle_task = 0;
static uint32_t next_pid = 1;
static uint32_t task_count = 0;

void task_init(void) {
    task_list_head = 0;
    current_task = 0;
    next_pid = 1;
    task_count = 0;
}

task_t *task_create(const char *name, void (*entry)(void), int priority) {
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    memset(task, 0, sizeof(task_t));

    task->pid = next_pid++;
    strncpy(task->name, name, TASK_NAME_MAX - 1);
    task->state = TASK_READY;
    task->priority = priority;
    task->brk = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;

    task->kernel_stack = (uint8_t *)kmalloc(TASK_STACK_SIZE);
    task->kernel_stack_top = (uint32_t)task->kernel_stack + TASK_STACK_SIZE;

    task->page_directory = mm_clone_directory();
    task->context.cr3 = task->page_directory->physical_addr;

    uint32_t *stack = (uint32_t *)(task->kernel_stack_top - sizeof(task_context_t));
    task_context_t *ctx = (task_context_t *)stack;
    memset(ctx, 0, sizeof(task_context_t));
    ctx->eip = (uint32_t)entry;
    ctx->eflags = 0x202;
    ctx->cs = 0x08;
    ctx->ss = 0x10;
    ctx->ds = 0x10;
    ctx->es = 0x10;
    ctx->fs = 0x10;
    ctx->gs = 0x10;
    task->context.esp = (uint32_t)stack;
    task->context.eip = (uint32_t)entry;

    task->next = task_list_head;
    task->parent = 0;
    task_list_head = task;
    task_count++;

    return task;
}

task_t *task_fork(void) {
    task_t *parent = current_task;
    if (!parent) return 0;

    task_t *child = (task_t *)kmalloc(sizeof(task_t));
    memcpy(child, parent, sizeof(task_t));

    child->pid = next_pid++;
    child->state = TASK_READY;
    child->parent = parent;
    child->page_directory = mm_clone_directory();
    child->context.cr3 = child->page_directory->physical_addr;
    child->kernel_stack = (uint8_t *)kmalloc(TASK_STACK_SIZE);
    memcpy(child->kernel_stack, parent->kernel_stack, TASK_STACK_SIZE);
    child->kernel_stack_top = (uint32_t)child->kernel_stack + TASK_STACK_SIZE;

    child->next = task_list_head;
    task_list_head = child;
    task_count++;

    return child;
}

void task_exit(int code) {
    if (!current_task) return;
    current_task->state = TASK_ZOMBIE;
    current_task->exit_code = code;
    task_schedule();
}

void task_yield(void) {
    task_schedule();
}

void task_sleep(uint32_t ticks) {
    if (!current_task) return;
    current_task->sleep_ticks = ticks;
    current_task->state = TASK_SLEEPING;
    task_schedule();
}

void task_wake(task_t *task) {
    if (task->state == TASK_SLEEPING) {
        task->state = TASK_READY;
        task->sleep_ticks = 0;
    }
}

void task_schedule(void) {
    task_t *old_task = current_task;
    task_t *next_task = 0;

    task_t *task = task_list_head;
    while (task) {
        if (task->state == TASK_READY || task->state == TASK_RUNNING) {
            if (!next_task || task->priority > next_task->priority) {
                next_task = task;
            }
        }
        if (task->state == TASK_SLEEPING && task->sleep_ticks > 0) {
            task->sleep_ticks--;
            if (task->sleep_ticks == 0) {
                task->state = TASK_READY;
            }
        }
        task = task->next;
    }

    if (!next_task) {
        next_task = idle_task;
        if (!next_task) {
            vga_puts("No tasks to run. Halting.\n");
            for (;;) asm volatile("hlt");
        }
    }

    if (next_task == current_task) return;

    next_task->state = TASK_RUNNING;
    if (current_task && current_task->state == TASK_RUNNING) {
        current_task->state = TASK_READY;
    }

    task_t *prev = current_task;
    current_task = next_task;
    mm_switch_page_directory(next_task->page_directory);

    if (prev) {
        task_switch_context(&prev->context, &next_task->context);
    }
}

task_t *task_get_current(void) {
    return current_task;
}

task_t *task_get_list_head(void) {
    return task_list_head;
}

task_t *task_get_by_pid(uint32_t pid) {
    task_t *task = task_list_head;
    while (task) {
        if (task->pid == pid) return task;
        task = task->next;
    }
    return 0;
}

void task_list(void) {
    task_t *task = task_list_head;
    vga_puts("PID   NAME                STATE     PRI\n");
    vga_puts("----  ------------------- --------- ---\n");
    while (task) {
        char buf[80];
        char *states[] = {"RUN", "RDY", "BLK", "SLP", "ZMB", "STP"};
        vga_puts(itoa(task->pid, buf, 10));
        vga_puts("   ");
        vga_puts(task->name);
        vga_puts("  ");
        vga_puts(states[task->state]);
        vga_puts("  ");
        vga_puts(itoa(task->priority, buf, 10));
        vga_puts("\n");
        task = task->next;
    }
}