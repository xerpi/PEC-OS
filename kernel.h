#ifndef KERNEL_H
#define KERNEL_H

#define NUM_TASKS 10

struct task_struct {
	union {
		struct {
			unsigned int r0;
			unsigned int r1;
			unsigned int r2;
			unsigned int r3;
			unsigned int r4;
			unsigned int r5;
			unsigned int r6;
			unsigned int r7;
			unsigned int pc;
			unsigned int psw;
		} reg;
		unsigned int regs[10];
	};
	int pid;
};

extern struct task_struct *current;

#endif
