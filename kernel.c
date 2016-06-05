#include "kernel.h"

static struct task_struct tasks[NUM_TASKS];


void rsi_timer()
{

}

void rsi_keys()
{

}

void rsi_switches()
{

}

void rsi_keyboard()
{

}

int kernel_main()
{
	void (*user_entry)() = (void (*)())0x1000;

	user_entry();

	return 0;
}
