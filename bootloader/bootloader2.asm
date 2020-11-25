[bits 16]
[org 0x7e00]

start:
    mov [bootdev], dl

GetVesaControllerInfo:
	;Preset 1st 4 bytes of block with 
	; 'VBE2' to get VBE2 information
	
	mov di, VESAControllerInfo
	mov byte [di], 'V'
	mov byte [di+1], 'B'
	mov byte [di+2], 'E'
	mov byte [di+3], '2'
	
	;Function 00h: get VESA controller information
	
	mov ax, 0x4f00
	int 10h
	
	;Get pointer to list of supported modes
	mov ax, word [VESAControllerInfo+14]
	
	push ax
	shr ax, 4	;Grab segment of pointer
	mov bx, ax	;and save it
	
	pop ax
	and ax, 0x000f
	
	mov dx, word [VESAControllerInfo+16]	;pointer's offset
	add dx, ax
	
	push es		;Save 'es' on stack
	mov es, bx	;Segment
	mov di, dx	;Offset
	; mov si, VESAControllerInfo	;Print VESA
	; call print
	
save_available_video_modes:
	mov bx, VbeModeList

.save_modes_loop:	;Save all available modes
	mov ax, word [es:di]
	
	cmp ax, 0xffff	;End of list?
	je finished_mode_save

	mov [bx], ax	;Save mode number
	
	add bx, 2
	add di, 2
	jmp .save_modes_loop
	
finished_mode_save:
	mov [bx], ax
	pop es
    jmp short do_e820

mmap_ent equ mem_map_entries_count             ; the number of entries will be stored at mem_map_entries_count

do_e820:
	pusha
	push es
	push bp

	mov ax, 0x3000
	mov es, ax
	mov di, 0        
	xor ebx, ebx				; ebx must be 0 to start
	xor bp, bp					; keep an entry count in bp
	mov edx, 0x0534D4150		; Place "SMAP" into edx
	mov eax, 0x0000e820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc short .failed			; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150		; Some BIOSes apparently trash this register?
	cmp eax, edx				; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx				; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin

.e820lp:
	mov eax, 0xe820				; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24					; ask for 24 bytes again
	int 0x15
	jc short .e820f				; carry set means "end of list already reached"
	mov edx, 0x0534D4150		; repair potentially trashed register

.jmpin:	
	jcxz .skipent				; skip any 0 length entries
	cmp cl, 20					; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent

.notext:
	mov ecx, [es:di + 8]		; get lower uint32_t of memory region length
	or ecx, [es:di + 12]		; "or" it with upper uint32_t to test for zero
	jz .skipent					; if length uint64_t is 0, skip entry
	inc bp						; got a good entry: ++count, move to next storage spot
	add di, 24

.skipent:
	test ebx, ebx				; if ebx resets to 0, list is complete
	jne short .e820lp

.e820f:
	mov [mmap_ent], bp			; store the entry count
	clc							; there is "jc" on end of list to this point, so the carry must be cleared

	mov dword [mem_map], 0x30000	; the memory map is stored at this address

	pop bp
	pop es
	popa
	jmp short load_kernel

.failed:
	mov si, memory_error
	call print
	jmp $

load_kernel:

	mov ah, 8
	mov dl, [bootdev]
	int 13h		;Get drive params

	jc disk_error

	and cx, 111111b
	mov [sectorsPerTrack], cx
	mov [numHeads], dh
	inc word [numHeads]

    push es
	mov ax, 0x900
	mov es, ax

	stc
	mov dh, 0
	mov ah, 0x02
	mov al, 120	;load 120 sectors
	mov ch, 0
	mov cl, 4	;Kernel is at the fourth sector of the disk
	
	mov dl, [bootdev]

	xor bx, bx ; [es:bx] = 0x0900:0x0000
	int 13h

	jc disk_error

	mov ax, 0x1800
	mov es, ax
	stc
	mov ax, 123
	call getchs
	mov dh, [head]
	mov ah, 0x02
	mov al, 120		;load another 120 sectors making a total of 240 sectors
	mov ch, [cyl]
	mov cl, [sect]
	
	mov dl, [bootdev]
	xor bx, bx		;[es:bx] = 0x1800:0x0000
	
	int 13h

	jnc switch_video_mode

disk_error:
	mov si, disk_read_error_msg
	call print
	xor ax, ax
	int 16h
	xor ax, ax
	int 19h

switch_video_mode:
    pop es
	mov di, VbeModeList
	
.loop:
	mov ax, word [es:di]
	cmp ax, 0xFFFF
	je next_mode

	mov cx, ax	;Save mode number in cx
	push di
	mov di, VbeModeInfo
	mov ax, 4f01h
	int 10h

	;This searches for 32bpp 1024*768 <Default OS Mode>
	cmp byte [di+25], 0x20    ;Bits Per Pixel
	jne .continue
	cmp WORD [di+18], 1024
	jne .continue
	cmp WORD [di+20], 768
	je found_mode

.continue:
	pop di
	add di, 2
	jmp .loop

found_mode:
	mov [CurrentMode], cx
	
	mov di, VbeModeInfo
	
	mov ax, 0x4f01	
	int 10h

	mov bx, cx
	mov ax, 0x4f02	;Set vbe mode

	int 10h

	jmp EnablePaging 
	
jmp $

next_mode:
	mov di, VbeModeList
	
.loop:
	mov ax, word [es:di]
	cmp ax, 0xFFFF
	je no_mode_found

	mov cx, ax	;Save mode number in cx
	push di
	mov di, VbeModeInfo
	mov ax, 4f01h
	int 10h

	;This searches for 32bpp 1280 * 1024

	cmp byte [di+25], 0x20    ;Bits Per Pixel
	jne .continue
	cmp WORD [di+18], 1280
	jne .continue
	cmp WORD [di+20], 1024
	je found_mode

.continue:
	pop di
	add di, 2
	jmp .loop

;For now, only two modes are supported in this OS
	
no_mode_found:
	mov si, no_mode_msg
	call print
	jmp $

;Print string routine
print:
    pusha

.loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0e
    int 10h
    jmp .loop

.done:
    popa
    ret

;GETCHS Function
;put lba value to convert in ax 
;returns cyl, head, sect

cyl db 0
head db 0
sect db 0
sectorsPerTrack dw 0
numHeads dw 0

getchs:
	xor dx,dx
	mov bx,[sectorsPerTrack]
	div bx
	inc dx
	mov byte [sect],dl
	xor dx,dx
	mov bx,[numHeads]
	div bx
	mov byte [cyl],al
	mov byte [head],dl
	ret

jmp $

EnablePaging:

.fill_page_directory:
	push es
	mov ax, 0x2900
	mov es, ax
	mov cx, 1024
	mov di, 0

.paging_loop0:	;Fill the page directory with zeros
	mov dword [es:di], 0x00000002
	add di, 4	
	loop .paging_loop0

;Fill the first two page table to map the first 8MB of memory.
; The remaining tables will be filled once the switch to protected mode is made and more memory 
;  can be accessed.
.fill_first_page_table:	 
	mov ax, 0x2A00 
	mov es, ax
	mov cx, 1024
	mov di, 0x0000
	mov esi, 0x00000003

.paging_loop1:
	mov dword [es:di], esi
	add di, 4
	add esi, 0x1000
	loop .paging_loop1

.fill_second_page_table:
	mov ax, 0x2B00
	mov es, ax
	mov cx, 1024
	mov di, 0
	mov esi, 0x00400003

.paging_loop2:
	mov dword [es:di], esi
	add di, 4
	add esi, 0x1000
	loop .paging_loop2

loadPageTables:	;Map the first 8mb
	mov ax, 0x2900
	mov es, ax
	mov di, 0x0000

	mov dword [es:di], 0x2A003	;Page directory entry 0 (0-3.99MB)
	add di, 4
	mov dword [es:di], 0x2B003	;Page directory entry 1 (4-7.99MB)

loadGDT:
	lgdt [gdt_descriptor]		;Load the Global Descriptor Table
	
loadPageDirectory:
	pop es
	mov dword [page_directory], 0x29000	;Save the linear address of the page directory

	mov eax, 0x29000	;Load the page directory address to cr3
	mov cr3, eax

switch_to_pmode:
	cli

	mov eax, cr0
	or eax, 0x80000001	;Turn on paging (bit 31) and protected mode enable (bit 0)
	mov cr0, eax
	
	jmp CODE_SEG:BEGIN_PM

;Data

;THE GLOBAL DESCRIPTOR TABLE
gdt_start:
	dd 0x0
	dd 0x0
gdt_code:
	dw 0xffff
	dw 0x0
	db 0x0
	db 0x9a
	db 11001111b
	db 0x0
gdt_data:
	dw 0xffff
	dw 0x0
	db 0x0
	db 0x92
	db 11001111b
	db 0x0
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

disk_read_error_msg: db "Error Loading OS Kernel. Press any key to reboot! Code: 0x01", 0
memory_error db "Memory test error! Reboot the PC to try again. Code: 0x03" ,0
no_mode_msg db 'No supported video mode found. Code: 0x04' , 0
bootdev db 0

[BITS 32]
BEGIN_PM:
	;Set up segments
	
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov ss, ax		;Set up stack segment
	mov ebp, 0x1F0000	;and pointer
	mov esp, ebp

	mov ecx, 512*240	;We have 240 sectors loaded
	mov esi, 0x9000		;at address 0x9000
	mov edi, 0x100000	;we must copy them to 0x100000

	rep movsb	;perform copy from EDI to ESI

	; Map the whole 4Gb 0f memory
fillPageTables:
	mov esi, 0x200000
	mov ecx, 1048576	;1024 page directory entries * 1024 entries per page table
	mov eax, 0x00000003

.paging_loop3:
	mov dword [esi], eax
	add esi, 4
	add eax, 0x1000
	loop .paging_loop3

.loadAllPageTables:
	mov ecx, 1024
	mov eax, 0x200003
	mov esi, 0x29000

.paging_loop4:
	mov dword [esi] ,eax
	add eax, 0x1000
	add esi, 4
	loop .paging_loop4

;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov eax, kernel_info_block
	call 0x100000	;Jump to kernel's entry point
	
_stop:
	hlt
	jmp _stop

times 1024 - ($-$$) db 0

section .bss
kernel_info_block:
    VESAControllerInfo: resb 512	;0x8200
	VbeModeList: resw 128	;0x8400
	CurrentMode: resw 1		;0x8500
	VbeModeInfo: resb 256	;0x8502
	mem_map_entries_count: resd 1	;0x8602
	mem_map: resd 1  			;0x8606
	page_directory: resd 1; 0x860A