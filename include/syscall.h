#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_EXIT      1
#define SYS_FORK      2
#define SYS_READ      3
#define SYS_WRITE     4
#define SYS_OPEN      5
#define SYS_CLOSE     6
#define SYS_WAITPID   7
#define SYS_CREAT     8
#define SYS_LINK      9
#define SYS_UNLINK    10
#define SYS_EXECVE    11
#define SYS_CHDIR     12
#define SYS_TIME      13
#define SYS_MKNOD     14
#define SYS_CHMOD     15
#define SYS_LCHOWN    16
#define SYS_BRK       45
#define SYS_GETPID    20
#define SYS_GETUID    24
#define SYS_GETEUID   49
#define SYS_GETGID    47
#define SYS_GETEGID   50
#define SYS_KILL      37
#define SYS_RENAME    38
#define SYS_MKDIR     39
#define SYS_RMDIR     40
#define SYS_DUP       41
#define SYS_PIPE      42
#define SYS_IOCTL     54
#define SYS_FCNTL     55
#define SYS_GETDENTS  141
#define SYS_STAT      106
#define SYS_LSEEK     19

void syscall_init(void);
int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx,
                    uint32_t edx, uint32_t esi, uint32_t edi);

#endif