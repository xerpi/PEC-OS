.include "macros.s"

	.set KERNEL_STACK_ADDR, _kernel_data_end

	.text
	.global _start
_start:
	; Setup interrupt/exception handler
	$movei r0, GSR_handler
	wrs s5, r0

	; Setup kernel stack
	$movei r7, KERNEL_STACK_ADDR

	$call r5, kernel_main

	halt

GSR_handler:
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
	;$call r5, RSG_routine

	; Get the exception ID
	rds r0, s2

	; Exception ID 14 = calls
	movi r1, 14
	cmpeq r3, r0, r1
	bnz r3, syscall_handler

	; Exception ID 15 = interrupt
	movi r1, 15
	cmpeq r3, r0, r1
	bnz r3, interrupt_handler

exception_handler:
	$movei r1, exception_vector_start
	; ID = 2 * ID
	add r0, r0, r0
	; r1 = &exception_vector_start[2 * ID]
	add r1, r1, r0
	ld r1, 0(r1)
	jal r5, r1
	bz r3, GSR_end

interrupt_handler:
	$movei r1, interrupt_vector_start
	getiid r0
	add r0, r0, r0
	add r1, r1, r0
	ld r1, 0(r1)
	jal r5, r1
	bnz r3, GSR_end

syscall_handler:
	; Get syscall number
	rds r0, s3

	; Check if the syscall is valid
	$movei r1, (syscall_table_end - syscall_table_start) / 2
	cmplt r2, r0, r1
	bz r2, invalid_syscall

	; Get syscall arguments
	and r1, r7, r7
	$movei r2, current
	ld r2, 0(r2)
	addi r7, r2, 2
	$pop r2, r3, r4, r5
	and r7, r1, r1
	$push r5, r4, r3, r2

	; Jump to the service
	$movei r1, syscall_table_start
	add r0, r0, r0
	add r1, r1, r0
	ld r1, 0(r1)
	jal r5, r1

	; Return the value to user (r1)
	$movei r0, current
	ld r0, 0(r0)
	st 2(r0), r1

	movi r0, 0
	bz r0, GSR_end

invalid_syscall:
	; Return -1 to user
	$movei r0, current
	ld r0, 0(r0)
	$movei r1, -1
	st 2(r0), r1

GSR_end:
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

	.global cpu_idle
cpu_idle:
	movi r0, 0
	bz r0, cpu_idle

ESR_default_halt:
	halt
ESR_default_resume:
	jmp r6

	.section .rodata
	.balign 2
exception_vector_start:
	.word ESR_default_halt   ; 0 Illegal instruction
	.word ESR_default_halt   ; 1 Unaligned access
	.word ESR_default_resume ; 2 Floating point overflow
	.word ESR_default_resume ; 3 Floating point division by zero
	.word ESR_default_halt   ; 4 Division by zero
	.word ESR_default_halt   ; 5 Undefined
	.word ESR_default_halt   ; 6 ITLB miss
	.word ESR_default_halt   ; 7 DTLB miss
	.word ESR_default_halt   ; 8 ITLB invalid page
	.word ESR_default_halt   ; 9 DTLB invalid page
	.word ESR_default_halt   ; 10 ITLB protected page
	.word ESR_default_halt   ; 11 DTLB protected page
	.word ESR_default_halt   ; 12 Read-only page
	.word ESR_default_halt   ; 13 Protection exception
exception_vector_end:

interrupt_vector_start:
	.word timer_routine
	.word key_routine
	.word switch_routine
	.word kb_routine
interrupt_vector_end:

syscall_table_start:
	.word sys_fork
	.word sys_getpid
syscall_table_end:
