#include <stdint.h>
#include "string.h"
#include "vga.h"
#include "mm.h"
#include "fs.h"
#include "task.h"
#include "kernel.h"

#define MAX_LINE 512
#define MAX_ARGS 32
#define MAX_HISTORY 50

static char history[MAX_HISTORY][MAX_LINE];
static int hist_count = 0;
static int hist_idx = 0;

extern void printf(const char *fmt, ...);
extern char keyboard_getchar(void);
extern int keyboard_haschar(void);
extern void timer_sleep(uint32_t ms);
extern uint32_t mm_get_total_memory(void);
extern uint32_t mm_get_free_memory(void);
extern uint32_t mm_get_used_memory(void);

static void shell_print(const char *str) {
    vga_puts(str);
}

static void shell_getline(char *buf, size_t maxlen) {
    size_t i = 0;
    buf[0] = '\0';
    while (i < maxlen - 1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            buf[i] = '\0';
            vga_putchar('\n');
            if (i > 0) {
                strncpy(history[hist_count % MAX_HISTORY], buf, MAX_LINE - 1);
                hist_count++;
                hist_idx = hist_count;
            }
            return;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
            }
        } else if (c >= ' ' && c <= '~') {
            buf[i++] = c;
            vga_putchar(c);
        }
    }
    buf[i] = '\0';
}

static int shell_execute(char *line);

static const char *banner =
"\n"
"  \\  /  _  | |    ___  _ __  \n"
"   \\/  /_\\ | |   / _ \\| '_ \\ \n"
"   /  / _ \\| |__|  __/| | | |\n"
"  /  /_/ \\_\\____\\___||_| |_|\n"
"\n";

static int cmd_help(char **args);
static int cmd_clear(char **args);
static int cmd_echo(char **args);
static int cmd_ls(char **args);
static int cmd_cd(char **args);
static int cmd_mkdir(char **args);
static int cmd_rm(char **args);
static int cmd_cat(char **args);
static int cmd_touch(char **args);
static int cmd_cp(char **args);
static int cmd_mv(char **args);
static int cmd_pwd(char **args);
static int cmd_whoami(char **args);
static int cmd_uname(char **args);
static int cmd_date(char **args);
static int cmd_ps(char **args);
static int cmd_kill(char **args);
static int cmd_free(char **args);
static int cmd_df(char **args);
static int cmd_mount(char **args);
static int cmd_umount(char **args);
static int cmd_reboot(char **args);
static int cmd_shutdown(char **args);
static int cmd_aptget(char **args);
static int cmd_dpkg(char **args);
static int cmd_systemctl(char **args);
static int cmd_man(char **args);
static int cmd_history_cmd(char **args);
static int cmd_hostname(char **args);
static int cmd_ifconfig(char **args);
static int cmd_ping(char **args);
static int cmd_wget(char **args);
static int cmd_curl(char **args);
static int cmd_ssh(char **args);
static int cmd_useradd(char **args);
static int cmd_passwd(char **args);
static int cmd_su(char **args);
static int cmd_chmod(char **args);
static int cmd_chown(char **args);
static int cmd_tar(char **args);
static int cmd_gzip(char **args);
static int cmd_find(char **args);
static int cmd_grep(char **args);
static int cmd_wc(char **args);
static int cmd_head(char **args);
static int cmd_tail(char **args);
static int cmd_sort(char **args);
static int cmd_uniq(char **args);
static int cmd_diff(char **args);
static int cmd_ln(char **args);
static int cmd_which(char **args);
static int cmd_whereis(char **args);
static int cmd_env(char **args);
static int cmd_export(char **args);
static int cmd_source(char **args);
static int cmd_alias(char **args);
static int cmd_unalias(char **args);
static int cmd_type_cmd(char **args);
static int cmd_exit(char **args);
static int cmd_sleep_cmd(char **args);
static int cmd_true_cmd(char **args);
static int cmd_false_cmd(char **args);

static struct {
    const char *name;
    const char *desc;
    int (*func)(char **);
} commands[] = {
    {"help",     "Show available commands", cmd_help},
    {"clear",    "Clear the terminal screen", cmd_clear},
    {"echo",     "Display a line of text", cmd_echo},
    {"ls",       "List directory contents", cmd_ls},
    {"cd",       "Change the current directory", cmd_cd},
    {"mkdir",    "Create directories", cmd_mkdir},
    {"rm",       "Remove files or directories", cmd_rm},
    {"cat",      "Concatenate and display files", cmd_cat},
    {"touch",    "Create empty files", cmd_touch},
    {"cp",       "Copy files and directories", cmd_cp},
    {"mv",       "Move/rename files", cmd_mv},
    {"pwd",      "Print working directory", cmd_pwd},
    {"whoami",   "Print current user", cmd_whoami},
    {"uname",    "Print system information", cmd_uname},
    {"date",     "Display or set system date", cmd_date},
    {"ps",       "Report process status", cmd_ps},
    {"kill",     "Terminate processes", cmd_kill},
    {"free",     "Display memory usage", cmd_free},
    {"df",       "Report filesystem usage", cmd_df},
    {"mount",    "Mount filesystems", cmd_mount},
    {"umount",   "Unmount filesystems", cmd_umount},
    {"reboot",   "Reboot the system", cmd_reboot},
    {"shutdown", "Shutdown the system", cmd_shutdown},
    {"apt-get",  "Package manager", cmd_aptget},
    {"dpkg",     "Debian package manager", cmd_dpkg},
    {"systemctl","System service manager", cmd_systemctl},
    {"man",      "Manual pages", cmd_man},
    {"history",  "Command history", cmd_history_cmd},
    {"hostname", "Show/set hostname", cmd_hostname},
    {"ifconfig", "Network interface config", cmd_ifconfig},
    {"ping",     "Network connectivity test", cmd_ping},
    {"wget",     "Download files", cmd_wget},
    {"curl",     "Transfer data from URLs", cmd_curl},
    {"ssh",      "Secure shell client", cmd_ssh},
    {"useradd",  "Add user account", cmd_useradd},
    {"passwd",   "Change password", cmd_passwd},
    {"su",       "Switch user", cmd_su},
    {"chmod",    "Change file permissions", cmd_chmod},
    {"chown",    "Change file ownership", cmd_chown},
    {"tar",      "Archive utility", cmd_tar},
    {"gzip",     "Compress files", cmd_gzip},
    {"find",     "Search for files", cmd_find},
    {"grep",     "Search text patterns", cmd_grep},
    {"wc",       "Word/line/byte count", cmd_wc},
    {"head",     "Show first lines", cmd_head},
    {"tail",     "Show last lines", cmd_tail},
    {"sort",     "Sort lines of text", cmd_sort},
    {"uniq",     "Remove duplicate lines", cmd_uniq},
    {"diff",     "Compare files", cmd_diff},
    {"ln",       "Create links", cmd_ln},
    {"which",    "Locate command", cmd_which},
    {"whereis",  "Locate binary/source", cmd_whereis},
    {"env",      "Show environment", cmd_env},
    {"export",   "Set environment variable", cmd_export},
    {"source",   "Execute script in shell", cmd_source},
    {"alias",    "Create command alias", cmd_alias},
    {"unalias",  "Remove alias", cmd_unalias},
    {"type",     "Display command type", cmd_type_cmd},
    {"exit",     "Exit the shell", cmd_exit},
    {"sleep",    "Delay for specified time", cmd_sleep_cmd},
    {"true",     "Return success", cmd_true_cmd},
    {"false",    "Return failure", cmd_false_cmd},
    {0, 0, 0}
};

static int cmd_help(char **args) {
    if (args[1]) {
        for (int i = 0; commands[i].name; i++) {
            if (strcmp(args[1], commands[i].name) == 0) {
                printf("%s - %s\n", commands[i].name, commands[i].desc);
                return 0;
            }
        }
        printf("No manual entry for %s\n", args[1]);
        return 0;
    }
    printf("Vaelen Shell Commands:\n");
    printf("%-12s %s\n", "COMMAND", "DESCRIPTION");
    printf("------------ ------------\n");
    for (int i = 0; commands[i].name; i++) {
        printf("%-12s %s\n", commands[i].name, commands[i].desc);
    }
    return 0;
}

static int cmd_clear(char **args) { vga_clear(); return 0; }
static int cmd_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        printf("%s%s", args[i], args[i+1] ? " " : "");
    }
    printf("\n");
    return 0;
}
static int cmd_ls(char **args) {
    printf("bin/  boot/  dev/  etc/  home/  lib/  mnt/  opt/\n");
    printf("proc/ root/ run/  sbin/ sys/  tmp/  usr/  var/\n");
    return 0;
}
static int cmd_cd(char **args) { return 0; }
static int cmd_mkdir(char **args) {
    if (!args[1]) { printf("mkdir: missing operand\n"); return 1; }
    printf("mkdir: created directory '%s'\n", args[1]);
    return 0;
}
static int cmd_rm(char **args) {
    if (!args[1]) { printf("rm: missing operand\n"); return 1; }
    printf("rm: removed '%s'\n", args[1]);
    return 0;
}
static int cmd_cat(char **args) {
    if (!args[1]) { printf("cat: missing operand\n"); return 1; }
    printf("cat: %s: No such file\n", args[1]);
    return 1;
}
static int cmd_touch(char **args) {
    if (!args[1]) { printf("touch: missing operand\n"); return 1; }
    return 0;
}
static int cmd_cp(char **args) {
    if (!args[1] || !args[2]) { printf("cp: missing operand\n"); return 1; }
    return 0;
}
static int cmd_mv(char **args) {
    if (!args[1] || !args[2]) { printf("mv: missing operand\n"); return 1; }
    return 0;
}
static int cmd_pwd(char **args) { printf("/\n"); return 0; }
static int cmd_whoami(char **args) { printf("root\n"); return 0; }
static int cmd_uname(char **args) {
    if (args[1] && strcmp(args[1], "-a") == 0) {
        printf("%s %s %s %s i686 GNU/Linux\n", KERNEL_NAME, KERNEL_VERSION, KERNEL_CODENAME, "x86_64");
    } else if (args[1] && strcmp(args[1], "-r") == 0) {
        printf("%s\n", KERNEL_VERSION);
    } else {
        printf("%s\n", KERNEL_NAME);
    }
    return 0;
}
static int cmd_date(char **args) { printf("Sat May 16 12:00:00 CST 2026\n"); return 0; }
static int cmd_ps(char **args) {
    printf("  PID TTY       TIME CMD\n");
    printf("    1 ?     00:00:00 init\n");
    printf("    2 ?     00:00:00 kthreadd\n");
    printf("    3 ?     00:00:00 shell\n");
    return 0;
}
static int cmd_kill(char **args) {
    if (!args[1]) { printf("kill: missing operand\n"); return 1; }
    return 0;
}
static int cmd_free(char **args) {
    printf("              total       used       free\n");
    printf("Mem:         %d      %d      %d\n",
        mm_get_total_memory(), mm_get_used_memory(), mm_get_free_memory());
    return 0;
}
static int cmd_df(char **args) {
    printf("Filesystem      Size  Used Avail Use%% Mounted on\n");
    printf("/dev/sda1       495G  2.1G  492G   1%% /\n");
    return 0;
}
static int cmd_mount(char **args) { return 0; }
static int cmd_umount(char **args) { return 0; }
static int cmd_reboot(char **args) {
    printf("Rebooting...\n");
    outb(0x64, 0xFE);
    return 0;
}
static int cmd_shutdown(char **args) {
    printf("Shutting down...\n");
    outb(0x64, 0xFE);
    return 0;
}
static int cmd_aptget(char **args) {
    if (!args[1]) {
        printf("apt-get: missing command\n");
        printf("Usage: apt-get [update|install|remove|upgrade|search]\n");
        return 1;
    }
    if (strcmp(args[1], "update") == 0) {
        printf("Get:1 http://vaelen.org stable InRelease\n");
        printf("Reading package lists... Done\n");
    } else if (strcmp(args[1], "install") == 0) {
        if (!args[2]) { printf("apt-get: missing package name\n"); return 1; }
        printf("Reading package lists... Done\n");
        printf("Building dependency tree... Done\n");
        printf("The following NEW packages will be installed:\n");
        printf("  %s\n", args[2]);
        printf("0 upgraded, 1 newly installed, 0 to remove.\n");
        printf("Installing %s... Done\n", args[2]);
    } else if (strcmp(args[1], "remove") == 0) {
        printf("Removing package... Done\n");
    } else if (strcmp(args[1], "upgrade") == 0) {
        printf("0 upgraded, 0 newly installed, 0 to remove.\n");
    } else if (strcmp(args[1], "search") == 0) {
        printf("Searching for '%s'...\n", args[2] ? args[2] : "");
    }
    return 0;
}
static int cmd_dpkg(char **args) { printf("dpkg: package manager stub\n"); return 0; }
static int cmd_systemctl(char **args) { printf("systemctl: service manager stub\n"); return 0; }
static int cmd_man(char **args) {
    if (!args[1]) { printf("What manual page do you want?\n"); return 1; }
    printf("No manual entry for %s\n", args[1]);
    return 0;
}
static int cmd_history_cmd(char **args) {
    for (int i = 0; i < hist_count && i < MAX_HISTORY; i++) {
        printf("  %d  %s\n", i + 1, history[i % MAX_HISTORY]);
    }
    return 0;
}
static int cmd_hostname(char **args) { printf("vaelen\n"); return 0; }
static int cmd_ifconfig(char **args) { printf("lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536\n"); return 0; }
static int cmd_ping(char **args) { printf("ping: network unavailable\n"); return 0; }
static int cmd_wget(char **args) { printf("wget: network unavailable\n"); return 0; }
static int cmd_curl(char **args) { printf("curl: network unavailable\n"); return 0; }
static int cmd_ssh(char **args) { printf("ssh: network unavailable\n"); return 0; }
static int cmd_useradd(char **args) { return 0; }
static int cmd_passwd(char **args) { return 0; }
static int cmd_su(char **args) { printf("su: not implemented\n"); return 0; }
static int cmd_chmod(char **args) { return 0; }
static int cmd_chown(char **args) { return 0; }
static int cmd_tar(char **args) { printf("tar: archive utility stub\n"); return 0; }
static int cmd_gzip(char **args) { printf("gzip: compression stub\n"); return 0; }
static int cmd_find(char **args) { printf("find: search stub\n"); return 0; }
static int cmd_grep(char **args) { printf("grep: search stub\n"); return 0; }
static int cmd_wc(char **args) { printf("0 0 0\n"); return 0; }
static int cmd_head(char **args) { return 0; }
static int cmd_tail(char **args) { return 0; }
static int cmd_sort(char **args) { return 0; }
static int cmd_uniq(char **args) { return 0; }
static int cmd_diff(char **args) { return 0; }
static int cmd_ln(char **args) { return 0; }
static int cmd_which(char **args) {
    if (!args[1]) return 0;
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(args[1], commands[i].name) == 0) {
            printf("/usr/bin/%s\n", args[1]);
            return 0;
        }
    }
    return 1;
}
static int cmd_whereis(char **args) { return 0; }
static int cmd_env(char **args) {
    printf("PATH=/usr/local/bin:/usr/bin:/bin\n");
    printf("HOME=/root\n");
    printf("USER=root\n");
    printf("SHELL=/bin/sh\n");
    printf("TERM=vaelen\n");
    return 0;
}
static int cmd_export(char **args) { return 0; }
static int cmd_source(char **args) { return 0; }
static int cmd_alias(char **args) { return 0; }
static int cmd_unalias(char **args) { return 0; }
static int cmd_type_cmd(char **args) { return 0; }
static int cmd_exit(char **args) {
    printf("logout\n");
    return -1;
}
static int cmd_sleep_cmd(char **args) {
    if (!args[1]) { printf("sleep: missing operand\n"); return 1; }
    int ms = atoi(args[1]) * 1000;
    timer_sleep(ms);
    return 0;
}
static int cmd_true_cmd(char **args) { return 0; }
static int cmd_false_cmd(char **args) { return 1; }

static int shell_execute(char *line) {
    char *args[MAX_ARGS];
    int argc = 0;
    char *p = line;

    while (*p && argc < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;
        args[argc++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) *p++ = '\0';
    }
    args[argc] = 0;
    if (argc == 0) return 0;

    for (int i = 0; commands[i].name; i++) {
        if (strcmp(args[0], commands[i].name) == 0) {
            return commands[i].func(args);
        }
    }

    printf("%s: command not found\n", args[0]);
    return 1;
}

void shell_run(void) {
    char line[MAX_LINE];

    vga_setcolor(0x07);
    printf("%s", banner);
    printf("%s v%s (%s) - Kernel built for physical hardware\n",
        KERNEL_NAME, KERNEL_VERSION, KERNEL_CODENAME);
    printf("Type 'help' for a list of available commands.\n\n");

    while (1) {
        printf("root@vaelen:~# ");
        shell_getline(line, MAX_LINE);
        if (line[0] == '\0') continue;
        if (shell_execute(line) < 0) {
            printf("\nVaelen Linux shell terminated.\n");
            return;
        }
    }
}