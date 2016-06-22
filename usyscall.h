#ifndef USYSCALL_H
#define USYSCALL_H

#include "libc.h"

/* Syscalls ABI:
 *   r0-r3: arguments
 */

static inline int syscall0_i(uint16_t number)
{
	__asm__(
		"and r0, %0, %0\n\t"
		"calls r0\n\t"
		: : "r"(number)
	);
}

static inline int syscall1_i(uint16_t number, uint16_t arg0)
{
	__asm__(
		"and r0, %0, %0\n\t"
		"and r1, %1, %1\n\t"
		"calls r0\n\t"
		: : "r"(number), "r"(arg0)
	);
}

#define fork()      ((int)syscall0_i(SYSCALL_FORK))
#define getpid() ((int)syscall0_i(SYSCALL_GETPID))
#define getticks()  ((unsigned int)syscall0_i(SYSCALL_GETTICKS))
#define readkb()    ((char)syscall0_i(SYSCALL_READKB))

#endif
