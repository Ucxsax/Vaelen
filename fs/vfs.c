#include "kernel.h"
#include "fs.h"
#include "mm.h"
#include "string.h"
#include "vga.h"

static fs_node_t *fs_root_node = 0;
static fd_entry_t fd_table[FS_MAX_FDS];
static fs_mount_t *mount_points = 0;
static char current_path[FS_PATH_MAX] = "/";

void fs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    fs_root_node = fs_create_node("", FS_DIRECTORY, 0755, 0, 0, 0);
    mount_points = 0;

    fs_create_node("bin", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("etc", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("home", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("tmp", FS_DIRECTORY, 0777, 0, 0, fs_root_node);
    fs_create_node("usr", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("var", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("dev", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("proc", FS_DIRECTORY, 0555, 0, 0, fs_root_node);
    fs_create_node("lib", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("opt", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("sbin", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("sys", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("mnt", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
    fs_create_node("root", FS_DIRECTORY, 0700, 0, 0, fs_root_node);
    fs_create_node("boot", FS_DIRECTORY, 0755, 0, 0, fs_root_node);
}

fs_node_t *fs_root(void) {
    return fs_root_node;
}

fs_node_t *fs_create_node(const char *name, uint32_t type, uint32_t mask,
                           uint32_t uid, uint32_t gid, fs_node_t *parent) {
    fs_node_t *node = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));

    strncpy(node->name, name, FS_NAME_MAX - 1);
    node->mask = mask;
    node->uid = uid;
    node->gid = gid;
    node->flags = type;
    node->inode = (uint32_t)node;

    if (parent) {
        node->ptr = parent;
    }

    return node;
}

static fs_node_t *fs_find_in_dir(fs_node_t *dir, const char *name) {
    return 0;
}

fs_node_t *fs_traverse(const char *path) {
    if (!path || !*path) return fs_root_node;
    if (path[0] == '/' && path[1] == '\0') return fs_root_node;
    return 0;
}

int fs_open(const char *path, uint32_t flags) {
    for (int i = 0; i < FS_MAX_FDS; i++) {
        if (fd_table[i].node == 0) {
            fd_table[i].node = fs_traverse(path);
            fd_table[i].offset = 0;
            fd_table[i].flags = flags;
            return i;
        }
    }
    return -1;
}

int fs_close(int fd) {
    if (fd < 0 || fd >= FS_MAX_FDS) return -1;
    if (!fd_table[fd].node) return -1;
    fd_table[fd].node = 0;
    return 0;
}

uint32_t fs_read(int fd, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (fd < 0 || fd >= FS_MAX_FDS) return 0;
    if (!fd_table[fd].node) return 0;
    return 0;
}

uint32_t fs_write(int fd, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (fd < 0 || fd >= FS_MAX_FDS) return 0;
    if (!fd_table[fd].node) return 0;
    return 0;
}

int fs_mkdir(const char *path, uint32_t mask) {
    return 0;
}

int fs_unlink(const char *path) {
    return 0;
}

int fs_rename(const char *oldpath, const char *newpath) {
    return 0;
}

dirent_t *fs_readdir(int fd, uint32_t index) {
    return 0;
}

int fs_stat(const char *path, fs_node_t *node) {
    return 0;
}

int fs_chdir(const char *path) {
    return 0;
}

int fs_mount(const char *path, fs_node_t *root) {
    fs_mount_t *mount = (fs_mount_t *)kmalloc(sizeof(fs_mount_t));
    strncpy(mount->mountpoint, path, FS_PATH_MAX - 1);
    mount->root = root;
    mount->next = mount_points;
    mount_points = mount;
    return 0;
}