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

extern void _user_code_start;
extern void _user_code_end;
extern void _user_data_start;
extern void _user_data_end;
extern void _kernel_code_start;
extern void _kernel_code_end;
extern void _kernel_data_start;
extern void _kernel_data_end;

#define SRAM_SIZE 0xA000

#define TLB_NUM_ENTRIES 8
#define TLB_ENTRY_BIT_R (1 << 4)
#define TLB_ENTRY_BIT_V (1 << 5)
#define TLB_ENTRY_BIT_P (1 << 6)

#define KERNEL_CODE_PAGE_START 0xC
#define KERNEL_DATA_PAGE_START 0x8

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)

#define NUM_USER_CODE_PAGES ((((uintptr_t)&_user_code_end - (uintptr_t)&_user_code_start) + PAGE_SIZE - 1) / PAGE_SIZE)
#define NUM_USER_DATA_PAGES ((((uintptr_t)&_user_data_end - (uintptr_t)&_user_data_start) + PAGE_SIZE - 1) / PAGE_SIZE)

#define NUM_KERNEL_CODE_PAGES ((((uintptr_t)&_kernel_code_end - (uintptr_t)&_kernel_code_start) + PAGE_SIZE - 1) / PAGE_SIZE)
#define NUM_KERNEL_DATA_PAGES ((((uintptr_t)&_kernel_data_end - (uintptr_t)&_kernel_data_start) + PAGE_SIZE - 1) / PAGE_SIZE)
#define NUM_KERNEL_PAGES (NUM_KERNEL_CODE_PAGES + NUM_KERNEL_DATA_PAGES)

#define NUM_FREE_PAGES (SRAM_SIZE / PAGE_SIZE)

#define NUM_TASKS 10

#define PAGE_TYPE_UNUSED 0
#define PAGE_TYPE_CODE   1
#define PAGE_TYPE_DATA   2

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
	struct {
		uint16_t type : 2;
		uint16_t pfn  : 4;
		uint16_t vpn  : 4;
		uint16_t r    : 1;
		uint16_t v    : 1;
		uint16_t p    : 1;
	} map[NUM_FREE_PAGES];
};

#define FREE_FRAME 0
#define USED_FRAME 1

/* Memory functions */
void mm_init();
int mm_alloc_frame();
void mm_free_frame(uint8_t frame);

/* TLB functions */
void tlb_setup_for_kernel();
void tlb_setup_for_task(const struct task_struct *task);

#endif