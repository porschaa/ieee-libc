
.global _start
_start:
	subq $8, %rsp
	xor %rbp, %rbp
	pop %rax
	pop %rsi
	mov %rsp, %rdx
	andq $-16, %rsp
	lea main(%rip), %rdi
	call __load_main

