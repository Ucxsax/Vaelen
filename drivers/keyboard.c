#include "kernel.h"
#include "isr.h"
#include "vga.h"
#include "task.h"

static char kb_buffer[256];
static int kb_buf_head = 0;
static int kb_buf_tail = 0;
static int kb_buf_count = 0;

static const char kb_us[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const char kb_us_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

static int shift_pressed = 0;
static int caps_lock = 0;

static void keyboard_handler(registers_t *regs) {
    uint8_t scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode & 0x80) return;

    char c;
    if (shift_pressed ^ caps_lock) {
        c = (scancode < sizeof(kb_us_shift)) ? kb_us_shift[scancode] : 0;
    } else {
        c = (scancode < sizeof(kb_us)) ? kb_us[scancode] : 0;
    }

    if (c && kb_buf_count < 256) {
        kb_buffer[kb_buf_tail] = c;
        kb_buf_tail = (kb_buf_tail + 1) % 256;
        kb_buf_count++;
    }
}

void keyboard_init(void) {
    irq_install_handler(1, keyboard_handler);
}

char keyboard_getchar(void) {
    while (kb_buf_count == 0) {
        asm volatile("hlt");
    }
    char c = kb_buffer[kb_buf_head];
    kb_buf_head = (kb_buf_head + 1) % 256;
    kb_buf_count--;
    return c;
}

int keyboard_haschar(void) {
    return kb_buf_count > 0;
}