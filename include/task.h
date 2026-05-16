#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "mm.h"

#define TASK_MAX 256
#define TASK_STACK_SIZE 8192
#define TASK_NAME_MAX 32

typedef enum {
    TASK_RUNNING = 0,
    TASK_READY = 1,
    TASK_BLOCKED = 2,
    TASK_SLEEPING = 3,
    TASK_ZOMBIE = 4,
    TASK_STOPPED = 5,
} task_state_t;

typedef struct task_context {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
    uint32_t cr3;
    uint32_t ss, cs, ds, es, fs, gs;
} task_context_t;

typedef struct task {
    uint32_t pid;
    char name[TASK_NAME_MAX];
    task_state_t state;
    task_context_t context;
    page_directory_t *page_directory;
    uint8_t *kernel_stack;
    uint32_t kernel_stack_top;
    uint8_t *user_stack;
    uint32_t sleep_ticks;
    int priority;
    int exit_code;
    struct task *next;
    struct task *parent;
    uint32_t heap_start;
    uint32_t heap_end;
    uint32_t brk;
} task_t;

void task_init(void);
task_t *task_create(const char *name, void (*entry)(void), int priority);
task_t *task_fork(void);
void task_exit(int code);
void task_yield(void);
void task_sleep(uint32_t ticks);
void task_wake(task_t *task);
void task_schedule(void);
task_t *task_get_current(void);
task_t *task_get_by_pid(uint32_t pid);
task_t *task_get_list_head(void);
void task_list(void);
void task_switch_context(task_context_t *old_ctx, task_context_t *new_ctx);

#endif