global badness

SECTION .text
badness:
	push ebp
	mov ebp,esp

	mov esp,ebp
	pop ebp
	ret
