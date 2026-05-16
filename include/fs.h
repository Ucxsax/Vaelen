#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

#define FS_FLAG_READ   0x01
#define FS_FLAG_WRITE  0x02
#define FS_FLAG_APPEND 0x04
#define FS_FLAG_CREATE 0x08
#define FS_FLAG_TRUNC  0x10

#define FS_NAME_MAX 256
#define FS_PATH_MAX 1024
#define FS_MAX_FDS  256

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t)(struct fs_node *);
typedef void (*close_type_t)(struct fs_node *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *name);

typedef struct dirent {
    char name[FS_NAME_MAX];
    uint32_t inode;
    uint32_t type;
} dirent_t;

typedef struct fs_node {
    char name[FS_NAME_MAX];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    struct fs_node *ptr;
} fs_node_t;

typedef struct fd_entry {
    fs_node_t *node;
    uint32_t offset;
    uint32_t flags;
} fd_entry_t;

typedef struct fs_mount {
    char mountpoint[FS_PATH_MAX];
    fs_node_t *root;
    struct fs_mount *next;
} fs_mount_t;

void fs_init(void);
fs_node_t *fs_root(void);
int fs_open(const char *path, uint32_t flags);
int fs_close(int fd);
uint32_t fs_read(int fd, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t fs_write(int fd, uint32_t offset, uint32_t size, uint8_t *buffer);
int fs_mkdir(const char *path, uint32_t mask);
int fs_unlink(const char *path);
int fs_rename(const char *oldpath, const char *newpath);
dirent_t *fs_readdir(int fd, uint32_t index);
int fs_stat(const char *path, fs_node_t *node);
int fs_chdir(const char *path);
int fs_mount(const char *path, fs_node_t *root);
fs_node_t *fs_traverse(const char *path);
fs_node_t *fs_create_node(const char *name, uint32_t type, uint32_t mask,
                           uint32_t uid, uint32_t gid, fs_node_t *parent);

#endif