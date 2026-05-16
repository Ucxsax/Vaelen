[BITS 32]

global gdt_flush
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

global idt_flush
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

global task_switch_context
task_switch_context:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov eax, [ebp+8]
    mov [eax+0], eax
    mov [eax+4], ebx
    mov [eax+8], ecx
    mov [eax+12], edx
    mov [eax+16], esi
    mov [eax+20], edi

    mov edx, [ebp+12]

    mov eax, [edx+0]
    mov ebx, [edx+4]
    mov ecx, [edx+8]
    mov edx, [edx+12]
    mov esi, [edx+16]
    mov edi, [edx+20]

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret