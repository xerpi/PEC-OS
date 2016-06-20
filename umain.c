#include "usyscall.h"

int main()
{
	int pid = fork();

	if (pid == 0) {
		main1();
	} else if (pid > 0) {
		main2();
	}

	return 0;
}
