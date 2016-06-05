.include "macros.s"

	.set KERNEL_STACK, 0xFFFE

	.data
current: .word 0

	.text
	.global _start
_start:
	; Setup interrupt handler
	$movei r0, RSG_handler
	wrs s5, r0

	; Setup kernel stack
	$movei r7, KERNEL_STACK

	$call r5, kernel_main

	halt

RSG_handler:
	; Save global purpose registers to current
	wrs s4, r0
	$movei r0, current
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
	$movei r7, KERNEL_STACK

	; Handle the interrupt/exception

	; Get exception ID
	rds r1, s2

	movi r2, 15
	cmplt r3, r2, r2
	bz r3, exception

exception:
	$movei r2, exceptions_vector
	add r1, r1, r2
	add r2, r2, r1
	ld r2, 0(r2)
	jal r5, r2
	bnz r3, end_RSG

interrupt:
	getiid r1
	add r1, r1, r1
	$movei r2, interrupts_vector
	add r2, r2, r1
	ld r2, 0(r2)
	jal r5, r2

end_RSG:

	$movei r0, current

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



exceptions_vector:
	.word RSE_default_halt   ; 0 Illegal instruction
	.word RSE_default_halt   ; 1 Unaligned access
	.word RSE_default_resume ; 2 Floating point overflow
	.word RSE_default_halt   ; 3 Division by zero

RSE_default_halt: halt
RSE_default_resume: jmp r6

interrupts_vector:
	.word rsi_timer
	.word rsi_keys
	.word rsi_switches
	.word rsi_keyboard


