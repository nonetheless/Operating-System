[section .data] 
color1 db 1Bh, '[39;36m', 0 
    .len equ $ - color1
color2 db 1Bh, '[0;0m', 0 
    .len equ $ - color2

[section .text]
global _myprint
extern _printf
_syscall:             
    int     0x80            ;system call  
    ret  
_myprint:
    mov  edx, [esp+16]
    cmp  edx,1
    je   NEXT
    call _change_color1
NEXT:
    push dword [esp+12]
    push dword [esp+8]
    push dword 1
    mov  eax, 0x4
    call _syscall
    add esp, 12
    call _change_color2
    ret

_change_color1:
    push dword color1.len
    push dword color1
    push dword 1
    mov eax, 0x4
    call _syscall
    add esp, 12
    ret

_change_color2:
    push dword color2.len
    push dword color2
    push dword 1
    mov eax, 0x4
    call _syscall
    add esp, 12
    ret
