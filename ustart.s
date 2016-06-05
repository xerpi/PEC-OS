.include "macros.s"

	.text
	.global _start
_start:
	; Setup user stack
	$movei r7, _user_stack

	; Jump to main()
	$call r5, main

while:
	movi r0, 0
	bz r0, while
