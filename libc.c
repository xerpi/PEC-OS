#include "libc.h"

void *memcpy(void *destination, const void *source, size_t num)
{
	size_t i;

	for (i = 0; i < num; i++) {
		*((uint8_t *)destination + i) = *((uint8_t *)source + i);
	}

	return destination;
}

void *memset(void *ptr, int value, size_t num)
{
	size_t i;

	for (i = 0; i < num; i++) {
		*((uint8_t *)ptr + i) = (uint8_t)value;
	}

	return ptr;
}
