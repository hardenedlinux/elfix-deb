global badness

SECTION .text
badness:
	push rbp
	mov rbp,rsp
	mov rsp,rbp
	pop rbp
	ret
