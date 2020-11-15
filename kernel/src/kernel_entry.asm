[bits 32]

global _start
extern kernel_main

_start:
    push eax
    call kernel_main

_stop:
    cli
    jmp _stop