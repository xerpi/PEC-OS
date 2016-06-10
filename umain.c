#include "libc.h"
#include "usyscall.h"

static void printstr(const char *str)
{
	static uint8_t x = 0;
	static uint8_t y = 0;
	while (*str) {
		((uint8_t *)0xA000)[(x + y * 80) << 1] = *str;
		if (*str == '\n') {
			x = 0;
			y++;
		} else {
			x++;
		}
		if (x > 8) {
			x = 0;
			y++;
		}
		if (y > 2)
			y = 0;
		str++;
	}
}

int main()
{
	int pid = fork();

	if (pid == 0)
		((uint8_t *)0xA000)[0] = 'A';
	else
		((uint8_t *)0xA000)[2] = 'B';


	/*for (i = 0; i < sizeof(str)/sizeof(*str); i++) {
		((uint8_t *)0xA000)[i << 1] = str[i];
	}*/

	return 0;
}
