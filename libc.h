#ifndef LIBC_H
#define LIBC_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef char int8_t;
typedef int int16_t;
typedef unsigned int size_t;
typedef unsigned int uintptr_t;

void *memcpy(void *destination, const void *source, size_t num);
void *memset(void *ptr, int value, size_t num);

#endif
