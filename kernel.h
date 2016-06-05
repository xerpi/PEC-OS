#ifndef KERNEL_H
#define KERNEL_H

#include "libc.h"

/* Memory map (physical)
 * +--------+_0xFFFF   ^
 * |        |          |
 * |        |          | ROM
 * |        |          |
 * | KCODE  |_0xC000   ∨
 * |  VGA   |
 * |  VGA   |_0xA000   ^
 * |   ^    |          |
 * | KDATA  |_0x8000   |
 * |        |          |
 * |        |          |
 * |        |          |
 * |        |          | SRAM_ACCESSIBLE
 * |        |          |
 * |        |          |
 * |        |          |
 * | 0-page |_0x0000   v
 * +--------+
 */

/* Memory map (virtual)
 * +--------+_0xFFFF
 * | KSTACK |
 * |   v    |
 * |   ^    |
 * | KCODE  |_0xC000
 * |        |
 * |        |_0xA000
 * |   ^    |
 * | KDATA  |_0x8000
 * |        |
 * |        |
 * |        |
 * |        |
 * |   ^    |
 * | UDATA  |
 * | UCODE  |
 * | 0-page |_0x0000
 * +--------+
 */

#define SRAM_ACCESSIBLE_SIZE 0xA000
#define PAGE_SIZE 0x1000
#define NUM_PAGES_TOTAL (SRAM_ACCESSIBLE_SIZE/PAGE_SIZE)

extern void _user_code_start;
extern void _user_code_end;
extern void _user_data_start;
extern void _user_data_end;

#define NUM_USER_CODE_PAGES ((((uintptr_t)&_user_code_end - (uintptr_t)&_user_code_start) + PAGE_SIZE - 1) / PAGE_SIZE)
#define NUM_USER_DATA_PAGES ((((uintptr_t)&_user_data_end - (uintptr_t)&_user_data_start) + PAGE_SIZE - 1) / PAGE_SIZE)

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
	uint8_t pfn_start_code;
	uint8_t pfn_start_data;
};

extern struct task_struct *current;
extern uint8_t phys_mem[NUM_PAGES_TOTAL];

#endif
