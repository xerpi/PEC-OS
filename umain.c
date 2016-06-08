#include "libc.h"

int main()
{
	int i;
	static const char str[] = "Hello world from userspace!";

	for (i = 0; i < sizeof(str)/sizeof(*str); i++) {
		((uint8_t *)0xA000)[i << 1] = str[i];
	}

	return 0;
}
