; Compiling
; nasm -f elf code.asm -o code.o
; ld code.o -o code.exe
; (gcclib) gcc code.o -o code.exe

global _main

; externe funktionen gcclib
extern _printf
extern _scanf

; output buffer
section .data
	prompt db "prompt> ", 0x0
	fmt db "%s", 0x0

; input buffer
section .bss
	input resb 1000

; code
section .text

_main:
	push ebp
	mov ebp, esp

_begin:
	; printf("prompt> ")
	push prompt
	call _printf
	add esp, 4

	; scanf("%s", input)
	push input
	push fmt
	call _scanf
	add esp, 8

	; printf("%s", input)
	push input
	call _printf
	add esp, 4

	jmp _begin

_end:
	mov esp, ebp
	pop ebp
	ret
