#include "kernel.h"
#include "mm.h"
#include "string.h"
#include "vga.h"

static uint32_t *mem_bitmap = 0;
static uint32_t total_memory = 0;
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;

static page_directory_t *kernel_directory = 0;
static page_directory_t *current_directory = 0;

static mem_block_t *heap_start = 0;
static mem_block_t *heap_end = 0;
static uint32_t heap_placement = KERNEL_HEAP_START;

static void mm_set_bit(int bit) {
    mem_bitmap[bit / 32] |= (1 << (bit % 32));
}

static void mm_unset_bit(int bit) {
    mem_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

static int mm_test_bit(int bit) {
    return mem_bitmap[bit / 32] & (1 << (bit % 32));
}

static int mm_first_free(void) {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (!mm_test_bit(i)) return i;
    }
    return -1;
}

void mm_init(uint32_t mem_size, uint32_t *bitmap) {
    total_memory = mem_size;
    total_blocks = mem_size / PAGE_SIZE;
    mem_bitmap = bitmap;
    used_blocks = 0;

    uint32_t bitmap_size = total_blocks / 8;
    memset(mem_bitmap, 0xFF, bitmap_size);

    uint32_t kernel_blocks = (KERNEL_HEAP_START - KERNEL_BASE) / PAGE_SIZE;
    for (uint32_t i = 0; i < kernel_blocks; i++) {
        mm_unset_bit(i);
    }
    used_blocks = kernel_blocks;
}

void mm_init_paging(void) {
    kernel_directory = (page_directory_t *)kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physical_addr = (uint32_t)kernel_directory->tables_physical;

    for (int i = 0; i < 1024; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->tables_physical[i] = 0;
    }

    uint32_t addr = 0;
    for (uint32_t i = 0; i < used_blocks; i++) {
        page_t *page = mm_get_page(addr, 1, kernel_directory);
        mm_alloc_frame(page, 1, 1);
        addr += PAGE_SIZE;
    }

    for (uint32_t i = KERNEL_HEAP_START; i < KERNEL_HEAP_START + KERNEL_HEAP_SIZE; i += PAGE_SIZE) {
        page_t *page = mm_get_page(i, 1, kernel_directory);
        mm_alloc_frame(page, 1, 1);
    }

    isr_register_handler(14, 0);
    mm_switch_page_directory(kernel_directory);
}

void mm_switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" : : "r"(dir->physical_addr));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

page_directory_t *mm_clone_directory(void) {
    page_directory_t *dir = (page_directory_t *)kmalloc_ap(sizeof(page_directory_t), &dir->physical_addr);
    memset(dir, 0, sizeof(page_directory_t));

    for (int i = 0; i < 1024; i++) {
        if (!kernel_directory->tables[i]) continue;
        if (kernel_directory->tables[i] == current_directory->tables[i]) {
            dir->tables[i] = kernel_directory->tables[i];
            dir->tables_physical[i] = kernel_directory->tables_physical[i];
        } else {
            page_table_t *table = (page_table_t *)kmalloc_ap(sizeof(page_table_t), &dir->tables_physical[i]);
            memcpy(table, current_directory->tables[i], sizeof(page_table_t));

            page_table_t *old_table = current_directory->tables[i];
            for (int j = 0; j < 1024; j++) {
                if (old_table->pages[j].frame) {
                    page_t *new_page = &table->pages[j];
                    page_t *old_page = &old_table->pages[j];
                    uint32_t tmp_frame = old_page->frame;
                    old_page->frame = 0;
                    mm_alloc_frame(new_page, 0, old_page->rw);
                    uint32_t phys_addr = tmp_frame * PAGE_SIZE;
                    memcpy((void *)(new_page->frame * PAGE_SIZE), (void *)phys_addr, PAGE_SIZE);
                    old_page->frame = tmp_frame;
                }
            }
            dir->tables[i] = table;
        }
    }
    return dir;
}

page_t *mm_get_page(uint32_t addr, int make, page_directory_t *dir) {
    addr /= PAGE_SIZE;
    uint32_t table_idx = addr / 1024;
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[addr % 1024];
    } else if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t *)kmalloc_ap(sizeof(page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, PAGE_SIZE);
        dir->tables_physical[table_idx] = tmp | 0x7;
        return &dir->tables[table_idx]->pages[addr % 1024];
    }
    return 0;
}

void mm_alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame != 0) return;
    int idx = mm_first_free();
    if (idx == -1) kernel_panic("Out of memory");
    mm_set_bit(idx);
    page->present = 1;
    page->rw = is_writeable ? 1 : 0;
    page->user = is_kernel ? 0 : 1;
    page->frame = idx;
    used_blocks++;
}

void mm_free_frame(page_t *page) {
    uint32_t frame = page->frame;
    if (!frame) return;
    mm_unset_bit(frame);
    page->frame = 0;
    used_blocks--;
}

static int mm_find_smallest_hole(uint32_t size, uint8_t page_align, mem_block_t **hole) {
    mem_block_t *current = heap_start;
    while (current) {
        if (current->free && current->size >= size) {
            *hole = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void *kmalloc(uint32_t size) {
    return kmalloc_p(size, 0);
}

void *kmalloc_a(uint32_t size) {
    return kmalloc_ap(size, 0);
}

void *kmalloc_p(uint32_t size, uint32_t *phys) {
    return kmalloc_ap(size, phys);
}

void *kmalloc_ap(uint32_t size, uint32_t *phys) {
    if (!heap_start) {
        heap_start = (mem_block_t *)KERNEL_HEAP_START;
        heap_start->size = KERNEL_HEAP_SIZE - sizeof(mem_block_t);
        heap_start->free = 1;
        heap_start->next = 0;
        heap_start->prev = 0;
        heap_end = heap_start;
    }

    if (size == 0) return 0;

    mem_block_t *hole = 0;
    if (!mm_find_smallest_hole(size + sizeof(mem_block_t), 0, &hole)) {
        uint32_t old_placement = heap_placement;
        heap_placement += size + sizeof(mem_block_t);
        if (heap_placement > KERNEL_HEAP_START + KERNEL_HEAP_SIZE) {
            kernel_panic("Kernel heap exhausted");
        }
        mem_block_t *block = (mem_block_t *)old_placement;
        block->size = size;
        block->free = 0;
        block->next = 0;
        block->prev = heap_end;
        if (heap_end) heap_end->next = block;
        heap_end = block;
        if (phys) *phys = old_placement + sizeof(mem_block_t);
        return (void *)(old_placement + sizeof(mem_block_t));
    }

    if (hole->size >= size + sizeof(mem_block_t) + 8) {
        uint32_t remaining = hole->size - size - sizeof(mem_block_t);
        mem_block_t *new_block = (mem_block_t *)((uint32_t)hole + sizeof(mem_block_t) + size);
        new_block->size = remaining;
        new_block->free = 1;
        new_block->next = hole->next;
        new_block->prev = hole;
        if (hole->next) hole->next->prev = new_block;
        hole->next = new_block;
        hole->size = size;
    }

    hole->free = 0;
    if (phys) *phys = (uint32_t)hole + sizeof(mem_block_t);
    return (void *)((uint32_t)hole + sizeof(mem_block_t));
}

void kfree(void *ptr) {
    if (!ptr) return;
    mem_block_t *block = (mem_block_t *)((uint32_t)ptr - sizeof(mem_block_t));
    block->free = 1;

    if (block->prev && block->prev->free) {
        block->prev->size += block->size + sizeof(mem_block_t);
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }

    if (block->next && block->next->free) {
        block->size += block->next->size + sizeof(mem_block_t);
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
}

uint32_t mm_get_total_memory(void) { return total_memory; }
uint32_t mm_get_free_memory(void) { return (total_blocks - used_blocks) * PAGE_SIZE; }
uint32_t mm_get_used_memory(void) { return used_blocks * PAGE_SIZE; }