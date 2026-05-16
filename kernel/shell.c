#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "vga.h"

#define MAX_LINE 256
#define MAX_ARGS 16

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

extern size_t terminal_row;
extern size_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;

extern void terminal_putchar(char c);
extern void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y);

void shell_print(const char *str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}

void shell_println(const char *str) {
    shell_print(str);
    terminal_putchar('\n');
}

void shell_backspace() {
    if (terminal_column > 0) {
        terminal_column--;
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
}

void shell_getline(char *buf, size_t maxlen) {
    size_t i = 0;
    while (i < maxlen - 1) {
        uint8_t c;
        asm volatile ("inb $0x60, %0" : "=a"(c));
        if (c & 0x80) continue;
        
        switch (c) {
            case 0x0E:
                if (i > 0) {
                    i--;
                    shell_backspace();
                }
                break;
            case 0x1C:
                buf[i] = '\0';
                terminal_putchar('\n');
                return;
            case 0x39:
                if (i < maxlen - 1) {
                    buf[i++] = ' ';
                    terminal_putchar(' ');
                }
                break;
            default:
                if (c >= ' ' && c <= '~' && i < maxlen - 1) {
                    buf[i++] = c;
                    terminal_putchar(c);
                }
                break;
        }
    }
    buf[i] = '\0';
}

int shell_cd(char **args);
int shell_ls(char **args);
int shell_mkdir(char **args);
int shell_rm(char **args);
int shell_echo(char **args);
int shell_help(char **args);
int shell_clear(char **args);

char *commands[] = {
    "cd",
    "ls",
    "mkdir",
    "rm",
    "echo",
    "help",
    "clear",
    NULL
};

int (*command_funcs[])(char **) = {
    shell_cd,
    shell_ls,
    shell_mkdir,
    shell_rm,
    shell_echo,
    shell_help,
    shell_clear
};

int shell_help(char **args) {
    shell_println("Vaelen Shell Commands:");
    shell_println("  cd [dir]     - Change directory");
    shell_println("  ls           - List directory contents");
    shell_println("  mkdir dir    - Create directory");
    shell_println("  rm file      - Remove file");
    shell_println("  echo text    - Print text");
    shell_println("  help         - Show this help");
    shell_println("  clear        - Clear screen");
    return 1;
}

int shell_clear(char **args) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    return 1;
}

int shell_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        shell_print(args[i]);
        if (args[i+1] != NULL) {
            shell_print(" ");
        }
    }
    shell_println("");
    return 1;
}

int shell_cd(char **args) {
    shell_println("cd: directory change not yet implemented");
    return 1;
}

int shell_ls(char **args) {
    shell_println("bin/");
    shell_println("etc/");
    shell_println("home/");
    shell_println("lib/");
    shell_println("proc/");
    shell_println("tmp/");
    shell_println("usr/");
    return 1;
}

int shell_mkdir(char **args) {
    if (args[1] == NULL) {
        shell_println("mkdir: missing operand");
        return 1;
    }
    shell_print("mkdir: created directory '");
    shell_print(args[1]);
    shell_println("'");
    return 1;
}

int shell_rm(char **args) {
    if (args[1] == NULL) {
        shell_println("rm: missing operand");
        return 1;
    }
    shell_print("rm: removed '");
    shell_print(args[1]);
    shell_println("'");
    return 1;
}

int shell_execute(char *line) {
    char *args[MAX_ARGS];
    int argc = 0;
    char *token = line;
    
    while (*token && argc < MAX_ARGS - 1) {
        while (*token == ' ' || *token == '\t') token++;
        if (*token == '\0') break;
        args[argc++] = token;
        while (*token && *token != ' ' && *token != '\t') token++;
        if (*token) *token++ = '\0';
    }
    args[argc] = NULL;
    
    if (argc == 0) return 1;
    
    for (int i = 0; commands[i] != NULL; i++) {
        if (strcmp(args[0], commands[i]) == 0) {
            return command_funcs[i](args);
        }
    }
    
    shell_print(args[0]);
    shell_println(": command not found");
    return 1;
}

void shell_run(void) {
    char line[MAX_LINE];
    shell_println("Vaelen Shell v0.1");
    shell_println("Type 'help' for available commands.");
    
    while (1) {
        shell_print("vaelen@localhost:~$ ");
        shell_getline(line, MAX_LINE);
        shell_execute(line);
    }
}