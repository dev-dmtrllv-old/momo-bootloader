[bits 32]

global halt

halt:
	cli
	hlt
	jmp $
