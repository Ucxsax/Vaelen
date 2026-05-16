[BITS 32]

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x03
    dd -(0x1BADB002 + 0x03)

section .text
global _start
extern kernel_main
extern gdt_flush
extern idt_flush

_start:
    mov esp, stack_top
    push eax
    push ebx
    call kernel_main

global load_page_directory
load_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov cr3, eax
    mov esp, ebp
    pop ebp
    ret

global enable_paging
enable_paging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: