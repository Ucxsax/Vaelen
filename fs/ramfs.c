#include "kernel.h"
#include "fs.h"
#include "mm.h"
#include "string.h"

#define RAMFS_MAX_FILES 64

typedef struct ramfs_file {
    char name[FS_NAME_MAX];
    uint8_t *data;
    uint32_t size;
    uint32_t capacity;
    uint32_t type;
    struct ramfs_file *parent;
    struct ramfs_file *children[RAMFS_MAX_FILES];
    int child_count;
} ramfs_file_t;

static ramfs_file_t *ramfs_root = 0;

static ramfs_file_t *ramfs_find(ramfs_file_t *dir, const char *name) {
    for (int i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return 0;
}

static ramfs_file_t *ramfs_create(ramfs_file_t *parent, const char *name, uint32_t type) {
    if (parent->child_count >= RAMFS_MAX_FILES) return 0;
    ramfs_file_t *file = (ramfs_file_t *)kmalloc(sizeof(ramfs_file_t));
    memset(file, 0, sizeof(ramfs_file_t));
    strncpy(file->name, name, FS_NAME_MAX - 1);
    file->type = type;
    file->parent = parent;
    if (type != FS_DIRECTORY) {
        file->data = (uint8_t *)kmalloc(PAGE_SIZE);
        file->capacity = PAGE_SIZE;
    }
    parent->children[parent->child_count++] = file;
    return file;
}

void ramfs_init(void) {
    ramfs_root = (ramfs_file_t *)kmalloc(sizeof(ramfs_file_t));
    memset(ramfs_root, 0, sizeof(ramfs_file_t));
    strcpy(ramfs_root->name, "");
    ramfs_root->type = FS_DIRECTORY;

    ramfs_create(ramfs_root, "bin", FS_DIRECTORY);
    ramfs_create(ramfs_root, "etc", FS_DIRECTORY);
    ramfs_create(ramfs_root, "home", FS_DIRECTORY);
    ramfs_create(ramfs_root, "tmp", FS_DIRECTORY);
    ramfs_create(ramfs_root, "usr", FS_DIRECTORY);
    ramfs_create(ramfs_root, "var", FS_DIRECTORY);
    ramfs_create(ramfs_root, "dev", FS_DIRECTORY);
    ramfs_create(ramfs_root, "proc", FS_DIRECTORY);
    ramfs_create(ramfs_root, "lib", FS_DIRECTORY);
}