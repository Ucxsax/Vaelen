#ifndef MM_H
#define MM_H

#include <stdint.h>

#define PAGE_PRESENT  0x1
#define PAGE_RW       0x2
#define PAGE_USER     0x4
#define PAGE_ACCESSED 0x20
#define PAGE_DIRTY    0x40

typedef struct page {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];
    uint32_t tables_physical[1024];
    uint32_t physical_addr;
} page_directory_t;

typedef struct mem_block {
    uint32_t size;
    struct mem_block *next;
    struct mem_block *prev;
    int free;
} mem_block_t;

void mm_init(uint32_t mem_size, uint32_t *bitmap);
void mm_init_paging(void);
void mm_switch_page_directory(page_directory_t *dir);
page_directory_t *mm_clone_directory(void);
page_t *mm_get_page(uint32_t addr, int make, page_directory_t *dir);
void mm_alloc_frame(page_t *page, int is_kernel, int is_writeable);
void mm_free_frame(page_t *page);
void *kmalloc(uint32_t size);
void *kmalloc_a(uint32_t size);
void *kmalloc_p(uint32_t size, uint32_t *phys);
void *kmalloc_ap(uint32_t size, uint32_t *phys);
void kfree(void *ptr);
uint32_t mm_get_total_memory(void);
uint32_t mm_get_free_memory(void);
uint32_t mm_get_used_memory(void);

#endif