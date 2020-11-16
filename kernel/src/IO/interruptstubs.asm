[bits 32]
section .text

global _ZN9crystalos2IO16InterruptManager22IgnoreInterruptRequestEv
extern _ZN9crystalos2IO16InterruptManager15HandleInterruptEhj

%macro HandleInterruptRequest 1
global _ZN9crystalos2IO16InterruptManager26HandleInterruptRequest%1Ev
_ZN9crystalos2IO16InterruptManager26HandleInterruptRequest%1Ev:
    mov byte[interruptNumber], %1+0x20
    push 0
    jmp int_bottom
%endmacro

%macro HandleException 1
global _ZN9crystalos2IO16InterruptManager19HandleException%1Ev
_ZN9crystalos2IO16InterruptManager19HandleException%1Ev:
    mov byte[interruptNumber], %1
    jmp int_bottom
%endmacro

HandleException 0x00	;Division by zero
HandleException 0x01	
HandleException 0x02	
HandleException 0x03	
HandleException 0x04	
HandleException 0x05	
HandleException 0x06	
HandleException 0x07	
HandleException 0x08	
HandleException 0x09	
HandleException 0x0A	
HandleException 0x0B	
HandleException 0x0C	
HandleException 0x0D	
HandleException 0x0E	;Page Fault Exception
HandleException 0x0F	
HandleException 0x10	
HandleException 0x11	
HandleException 0x12	
HandleException 0x13	
HandleException 0x14	
HandleException 0x15	
HandleException 0x16	
HandleException 0x17	
HandleException 0x18	
HandleException 0x19	
HandleException 0x1A	
HandleException 0x1B	
HandleException 0x1C	
HandleException 0x1D	
HandleException 0x1E	
HandleException 0x1F	

HandleInterruptRequest 0x00		;Timer interrupt
HandleInterruptRequest 0x01		;Keyboard interrupt
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03		
HandleInterruptRequest 0x04		
HandleInterruptRequest 0x05		
HandleInterruptRequest 0x06		
HandleInterruptRequest 0x07		
HandleInterruptRequest 0x08		
HandleInterruptRequest 0x09		
HandleInterruptRequest 0x0A		
HandleInterruptRequest 0x0B		
HandleInterruptRequest 0x0C	
HandleInterruptRequest 0x0D	
HandleInterruptRequest 0x0E	
HandleInterruptRequest 0x0F		
HandleInterruptRequest 0x10		
HandleInterruptRequest 0x11		
HandleInterruptRequest 0x12		
HandleInterruptRequest 0x13		
HandleInterruptRequest 0x14		
HandleInterruptRequest 0x15		
HandleInterruptRequest 0x16	
HandleInterruptRequest 0x17		
HandleInterruptRequest 0x18		
HandleInterruptRequest 0x19		
HandleInterruptRequest 0x1A
HandleInterruptRequest 0x1B
HandleInterruptRequest 0x1C
HandleInterruptRequest 0x1D
HandleInterruptRequest 0x1E
HandleInterruptRequest 0x1F
HandleInterruptRequest 0x20
HandleInterruptRequest 0x21
HandleInterruptRequest 0x22
HandleInterruptRequest 0x23
HandleInterruptRequest 0x24
HandleInterruptRequest 0x25
HandleInterruptRequest 0x26
HandleInterruptRequest 0x27
HandleInterruptRequest 0x28
HandleInterruptRequest 0x29
HandleInterruptRequest 0x2A
HandleInterruptRequest 0x2B
HandleInterruptRequest 0x2C
HandleInterruptRequest 0x2D
HandleInterruptRequest 0x2E
HandleInterruptRequest 0x2F
HandleInterruptRequest 0x30


int_bottom:

    push ebp
	push edi
	push esi
	
	push edx
	push ecx
	push ebx
	push eax

    push esp
    push dword [interruptNumber]
    call _ZN9crystalos2IO16InterruptManager15HandleInterruptEhj
    mov esp, eax

    pop eax
    pop ebx
    pop ecx
    pop edx

    pop esi
    pop edi
    pop ebp

    add esp, 4

_ZN9crystalos2IO16InterruptManager22IgnoreInterruptRequestEv:
    iret

section .data
    interruptNumber db 0

