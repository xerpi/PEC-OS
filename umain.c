#include "libc.h"
#include "usyscall.h"

int main()
{
	int pid = fork();

	if (pid == 0) /* Child */
		((uint8_t *)0xA000)[0] = 'C';
	else if (pid > 0) /* Parent */
		((uint8_t *)0xA000)[2] = '0' + pid;
	else /* Error */
		((uint8_t *)0xA000)[4] = 'E';

	return 0;
}
