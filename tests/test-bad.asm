global badness:function

SECTION .text

align 16
badness:
	push rbp
	mov rbp,rsp

	mov rsp,rbp
	pop rbp
	ret
