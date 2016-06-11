#include "libc.h"
#include "usyscall.h"

char a = 0;

int main()
{
	int pid = fork();

	if (pid == 0) {
		while (1) {
			((uint8_t *)0xA000)[0] = '0' + a;
		}
	} else if (pid > 0) {
		while (1) {
			((uint8_t *)0xA000)[2] = '0' + a;
			a = (a+1) % 10;
		}
	} else {
		((uint8_t *)0xA000)[4] = 'E';
	}

	return 0;
}
