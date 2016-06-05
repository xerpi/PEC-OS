#ifndef ASMUTILS_H
#define ASMUTILS_H

#include "libc.h"

static inline __attribute__((always_inline)) void wrpi(uint8_t entry, uint8_t value)
{
	__asm__(
		"wrpi %0, %1\n\t"
		: "=r"(entry), "=r"(value)
	);
}

static inline __attribute__((always_inline)) void wrvi(uint8_t entry, uint8_t value)
{
	__asm__(
		"wrvi %0, %1\n\t"
		: "=r"(entry), "=r"(value)
	);
}

static inline __attribute__((always_inline)) void wrpd(uint8_t entry, uint8_t value)
{
	__asm__(
		"wrpd %0, %1\n\t"
		: "=r"(entry), "=r"(value)
	);
}

static inline __attribute__((always_inline)) void wrvd(uint8_t entry, uint8_t value)
{
	__asm__(
		"wrvd %0, %1\n\t"
		: "=r"(entry), "=r"(value)
	);
}

#endif
