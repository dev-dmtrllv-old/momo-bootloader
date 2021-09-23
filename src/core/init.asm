[bits 32]

extern main

section .text.init

global start
start:
	call main
	cli
	hlt
	jmp $
