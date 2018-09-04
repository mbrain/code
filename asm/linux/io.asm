; Compiling
; nasm -f elf32 code.asm -o code.o
; ld -m elf_i386 code.o -o code

global _start

section .bss

	buffer resb 10	; 10 byte buffer
	bufferlen equ $ - buffer

section .data

        prompt db "prompt> "	; , 0x0a
        promptlen equ $ - prompt

section .text

_start:

loop1:

        mov eax, 4	; sys_write
        mov ebx, 1 	; stdout
        mov ecx, prompt	; prompt
        mov edx, promptlen	; length predefined
        int 0x80

	mov eax, 3	; sys_read
	mov ebx, 0	; stdin
	mov ecx, buffer	; buffer
	mov edx, bufferlen	; length 10 bytes
	int 80h

	cmp eax, 0
	jle lpend1	; if read <= 0

	mov eax, 4	; sys_write
	mov ebx, 1	; stdout
	mov ecx, buffer	; buffer
	mov edx, bufferlen	; length
	int 80h

	jmp loop1

lpend1: 

	mov eax, 1	; sys_exit
	mov ebx, 0	; status
	int 80h
