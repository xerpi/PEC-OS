.include "macros.s"

	.set KERNEL_STACK_ADDR, _kernel_data_end

	.text
	.global _start
_start:
	; Setup interrupt handler
	$movei r0, RSG_handler
	wrs s5, r0

	; Setup kernel stack
	$movei r7, KERNEL_STACK_ADDR

	$call r5, kernel_main

	halt

RSG_handler:
	; Save global purpose registers to current
	wrs s4, r0
	$movei r0, current
	ld r0, 0(r0)
	st 2(r0), r1
	st 4(r0), r2
	st 6(r0), r3
	st 8(r0), r4
	st 10(r0), r5
	st 12(r0), r6
	st 14(r0), r7
	rds r1, s4
	st 0(r0), r1

	; Save pc to current
	rds r1, s1
	st 16(r0), r1

	; Save psw to current
	rds r1, s0
	st 18(r0), r1

	; Setup kernel stack
	$movei r7, KERNEL_STACK_ADDR

	; Jump to the C routine
	$call r5, RSG_routine

	; Restore context
	$movei r0, current
	ld r0, 0(r0)

	; Restore psw from current
	ld r1, 18(r0)
	wrs s0, r1

	; Restore pc from current
	ld r1, 16(r0)
	wrs s1, r1

	; Restore global purpose registers from current
	ld r7, 14(r0)
	ld r6, 12(r0)
	ld r5, 10(r0)
	ld r4, 8(r0)
	ld r3, 6(r0)
	ld r2, 4(r0)
	ld r1, 2(r0)
	ld r0, 0(r0)

	reti
