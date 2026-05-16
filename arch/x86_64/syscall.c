#include "kernel.h"
#include "syscall.h"
#include "fs.h"
#include "task.h"
#include "vga.h"
#include "string.h"
#include "mm.h"

static int sys_write(int fd, const char *buf, int len) {
    if (fd == 1 || fd == 2) {
        for (int i = 0; i < len; i++) {
            vga_putchar(buf[i]);
        }
        return len;
    }
    return fs_write(fd, 0, len, (uint8_t *)buf);
}

static int sys_read(int fd, char *buf, int len) {
    return fs_read(fd, 0, len, (uint8_t *)buf);
}

static int sys_open(const char *path, int flags) {
    return fs_open(path, flags);
}

static int sys_close(int fd) {
    return fs_close(fd);
}

static int sys_mkdir(const char *path, int mode) {
    return fs_mkdir(path, mode);
}

static int sys_unlink(const char *path) {
    return fs_unlink(path);
}

static int sys_getpid(void) {
    task_t *task = task_get_current();
    return task ? task->pid : 0;
}

static void *sys_brk(void *addr) {
    task_t *task = task_get_current();
    if (!task) return (void *)0;
    if (addr) {
        task->brk = (uint32_t)addr;
    }
    return (void *)task->brk;
}

static int sys_fork(void) {
    task_t *child = task_fork();
    if (child) {
        return child->pid;
    }
    return 0;
}

static void sys_exit(int code) {
    task_exit(code);
}

static int sys_time(void) {
    return 0;
}

static int sys_kill(int pid, int sig) {
    return -1;
}

int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx,
                    uint32_t edx, uint32_t esi, uint32_t edi) {
    switch (eax) {
        case SYS_WRITE:
            return sys_write((int)ebx, (const char *)ecx, (int)edx);
        case SYS_READ:
            return sys_read((int)ebx, (char *)ecx, (int)edx);
        case SYS_OPEN:
            return sys_open((const char *)ebx, (int)ecx);
        case SYS_CLOSE:
            return sys_close((int)ebx);
        case SYS_MKDIR:
            return sys_mkdir((const char *)ebx, (int)ecx);
        case SYS_UNLINK:
            return sys_unlink((const char *)ebx);
        case SYS_GETPID:
            return sys_getpid();
        case SYS_BRK:
            return (int)sys_brk((void *)ebx);
        case SYS_FORK:
            return sys_fork();
        case SYS_EXIT:
            sys_exit((int)ebx);
            return 0;
        case SYS_TIME:
            return sys_time();
        case SYS_KILL:
            return sys_kill((int)ebx, (int)ecx);
        default:
            return -1;
    }
}

void syscall_init(void) {
    isr_register_handler(0x80, 0);
}