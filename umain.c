#include "libc.h"
#include "usyscall.h"

char a = 0;

static void printstr(uint8_t row, uint8_t color, const char *str)
{
	uint8_t x = 0;
	uint8_t y = row;
	while (*str) {
		((uint8_t *)0xA000)[(x + y * 80) << 1] = *str;
		((uint8_t *)0xA000)[((x + y * 80) << 1) + 1] = color;
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
		str++;
	}
}

int main()
{
	int pid = fork();

	if (pid == 0) {
		while (1) {
			((uint8_t *)0xA000)[20] = '0' + a;
			((uint8_t *)0xA000)[21] = 3;
			printstr(0, 3, "Parent");
			
		}
	} else if (pid > 0) {
		pid = fork();
		if (pid == 0) {
			while (1) {
				((uint8_t *)0xA000)[20 + 80*1*2] = '0' + a;
				((uint8_t *)0xA000)[21 + 80*1*2] = 0x30;
				printstr(1, 0x30, "Child 1");
				a = (a+1) % 10;
			}
		} else {
			while (1) {
				((uint8_t *)0xA000)[20 + 80*2*2] = 'a' + a;
				((uint8_t *)0xA000)[21 + 80*2*2] = 0xC;
				printstr(2, 0xC, "Child 2");
				a = (a+1) % 10;
			}
		}
	}

	return 0;
}
