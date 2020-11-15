[bits 16]
[org 0x7c00]

start:
    ;Set up segments correctly
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;Set up stack
    mov ss, ax
    mov bp, 0x7c00
    mov sp, bp

    ;Save boot device number
    mov [bootdev], dl

;Enable A20 Gate
EnableA20Gate:
	call TestA20
	cmp ax, 1
	je A20Enabled
	
tryUsingBIOS:
	mov ax, 0x2401
	int 0x15
	call TestA20
	cmp ax, 1
	je A20Enabled
	
tryUsingKeyboardController:
	cli
	call WaitCommand
	mov al, 0xAD	;Disable the keyboard
	out 0x64, al
	
	call WaitCommand
	mov al, 0xD0	;Read from input
	out 0x64, al
	
	call WaitData
	in al, 0x60		;Read input from keyboard
	push ax			;Save it
	
	call WaitCommand
	mov al, 0xD1	;Write to output
	out 0x64, al
	
	call WaitCommand
	pop ax			;Write to input back with bit #2 set
	or al, 2
	out 0x60, al
	
	call WaitCommand
	mov al, 0xAE	;Enable Keyboard
	out 0x64, al
	
	call WaitCommand
	sti
	jmp A20KeyboardCheck
	
WaitCommand:
	in al, 0x64
	test al, 2
	jnz WaitCommand
	ret

WaitData:
	in al, 0x64
	test al, 1
	jz WaitData
	ret
	
A20KeyboardCheck:
	call TestA20
	cmp ax, 1
	je A20Enabled

UseFastA20Method:
	in al, 0x92
	or al, 2
	out 0x92, al
	call TestA20
	cmp ax, 1
	je A20Enabled
	
;Else bail out, A20 cannot be enabled, maybe :)
A20Error:
	mov si, A20_error_msg
	call print
	xor ax, ax
	int 16h
	xor ax, ax
	int 19h
	
jmp $
;If we ever get here, A20 is enabled!
A20Enabled:
LoadSecondStage:
	push es
	mov ax, 0x7e0
	mov es, ax

	stc
	mov dh, 0
	mov ah, 0x02
	mov al, 2	;load 2 sectors
	mov ch, 0
	mov cl, 2	
	
	mov dl, [bootdev]

	xor bx, bx ; [es:bx] = 0x07e0:0x0000
	int 13h

	jnc load_success

disk_error:
	mov si, disk_read_error_msg
	call print
	xor ax, ax
	int 16h
	xor ax, ax
	int 19h

jmp $

load_success:
	pop es
	mov dl, [bootdev]
	jmp 0x07e0:0x0000

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

;****************************
;Function to check if A20 Gate is enabled
;IN = nothing
;OUT : AX = status; 0 = Disabled, 1 = Enabled
TestA20:
	cli
	push es
	push di
	push ds
	push si
	
	push bx
	push dx
	
	xor dx, dx
	xor bx, bx
	
	mov es, bx
	mov di, 0x0500
	
	mov bx, 0xffff
	mov ds, bx
	mov si, 0x0510
	
	mov al, byte [es:di]
	push ax
	
	mov al, byte [ds:si]
	push ax
		
	mov [es:di], byte 0x00
	mov [ds:si], byte 0xff
	
	mov bl, byte [es:di]
	cmp bl, 0xff
	je A20Exit
	mov dx, 0x0001		;A20 Enabled
	
A20Exit:
	pop ax
	mov [ds:si], al
	pop ax
	mov [es:di], al
	
	mov ax, dx
	pop dx
	pop bx
	pop si
	pop ds
	pop di
	pop es
	sti
	ret
;;;;;;End of function;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

disk_read_error_msg db 'Error Reading disk. Press any key to reboot. Code : 0x01', 0
A20_error_msg db 'An Internal error occured. Press any key to reboot. Code: 0x02', 0
bootdev db 0

times 442 - ($-$$) db 0
dd 0x07112026	;Boot signature: Unique for each boot disk this OS is installed on  ;6:20 PM

;Written on 7th November, 2020 6:20 pm
;Partition tables
; #1

db 0x0
db 0
db 0
db 0
db 0x00	;Reserved
db 0
db 0
db 0
dd 0x0
dd 20480

; #2
db 0x80
db 0
db 0
db 0
db 07h	;FAT 32 fs
db 0
db 0
db 0
dd 20480
dd 8388608

; #3
times 16 db 0

; #4
times 16 db 0

dw 0xaa55