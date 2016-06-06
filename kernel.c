#include "kernel.h"
#include "asmutils.h"

struct task_struct *current;

static struct task_struct task[NUM_TASKS];

static uint8_t phys_mem[NUM_FREE_PAGES];

static struct list_head freequeue;
static struct list_head readyqueue;

static struct task_struct *idle_task;

static int last_pid;

static void RSE_routine(uint8_t exception_id)
{
	switch (exception_id) {
	default:
		break;
	}
}

static void RSI_routine()
{
	uint8_t interrupt_id;

	/* Get interrupt ID */
	__asm__(
		"getiid %0\n\t"
		: "=r"(interrupt_id)
	);

	switch (interrupt_id) {
	default:
		break;
	}
}

void RSG_routine()
{
	uint8_t exception_id;

	/* Get exception ID */
	__asm__(
		"rds %0, s2\n\t"
		: "=r"(exception_id)
	);

	if (exception_id < 15)
		RSE_routine(exception_id);
	else
		RSI_routine();
}

void mm_init()
{
	int i;
	for (i = 0; i < ARRAY_SIZE(phys_mem); i++) {
		phys_mem[i] = FREE_FRAME;
	}
}

int mm_alloc_frame()
{
	int i;
	for (i = 0; i < ARRAY_SIZE(phys_mem); i++) {
		if (phys_mem[i] == FREE_FRAME) {
			phys_mem[i] = USED_FRAME;
			return i;
		}
	}
	return -1;
}

void mm_free_frame(uint8_t frame)
{
	phys_mem[frame] = FREE_FRAME;
}

void tlb_setup_for_kernel()
{
	int i;
	int j = 0;

	/* Kernel code */
	for (i = 0; i < NUM_KERNEL_CODE_PAGES; i++, j++) {
		// ITLB
		wrpi(j, (KERNEL_CODE_PAGE_START + i) | TLB_ENTRY_BIT_R |
			TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvi(j, KERNEL_CODE_PAGE_START + i);
		// DTLB code read-only
		wrpd(j, (KERNEL_CODE_PAGE_START + i) | TLB_ENTRY_BIT_R |
			TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvd(j, KERNEL_CODE_PAGE_START + i);
	}

	/* Kernel data (and stack) */
	for (i = 0; i < NUM_KERNEL_DATA_PAGES; i++, j++) {
		wrpd(j, (KERNEL_DATA_PAGE_START + i) | TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvd(j, KERNEL_DATA_PAGE_START + i);
	}
}

void tlb_setup_for_task(const struct task_struct *task)
{
	int i, j;
	uint8_t pc_entry;
	uint8_t num_mapped_pages;

	/* From the entry 0 to NUM_KERNEL_PAGES we have the kernel */

	/* Find the page where the PC points to */
	for (i = 0; i < ARRAY_SIZE(task->map); i++) {
		if ((task->reg.pc >> PAGE_SHIFT) == task->map[i].vpn &&
		    task->map[i].v == 1 && task->map[i].type == PAGE_TYPE_CODE) {
			pc_entry = i;
			break;
		}
	}

	/* Map the page where the PC points to (just after the kernel map) */
	wrpi(NUM_KERNEL_PAGES, task->map[pc_entry].pfn | (task->map[pc_entry].r << 4)
		| (task->map[pc_entry].v << 5) | (task->map[pc_entry].p << 6));

	wrvi(NUM_KERNEL_PAGES, task->map[pc_entry].vpn);

	num_mapped_pages = NUM_KERNEL_PAGES + 1;

	/* Map as most remaining pages as we can */
	for (i = 0; i < ARRAY_SIZE(task->map) && num_mapped_pages < TLB_NUM_ENTRIES; i++) {
		if (task->map[i].type != PAGE_TYPE_UNUSED && i != pc_entry) {
			if (task->map[i].type == PAGE_TYPE_CODE) {
				wrpi(num_mapped_pages,
					task->map[i].pfn | (task->map[i].r << 4) |
					(task->map[i].v << 5) | (task->map[i].p << 6));

				wrvi(num_mapped_pages, task->map[i].vpn);

				num_mapped_pages++;
			} else if (task->map[i].type == PAGE_TYPE_DATA) {
				wrpd(num_mapped_pages,
					task->map[i].pfn | (task->map[i].r << 4) |
					(task->map[i].v << 5) | (task->map[i].p << 6));

				wrvd(num_mapped_pages, task->map[i].vpn);

				num_mapped_pages++;
			}
		}
	}
}

int get_free_PID()
{
	return ++last_pid;
}

void cpu_idle(void)
{
	while(1)
		;
}

void set_user_pages(struct task_struct *ts) {

}

void init_queues()
{
	int i;

	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);

	for (i = 0; i < NUM_TASKS; i++)
		list_add_tail(&(&task[i])->list, &freequeue);
}

void init_idle(void)
{
	idle_task = (struct task_struct *)list_pop_front(&freequeue);

	idle_task->pid = 0;
	idle_task->reg.pc = (uintptr_t)&cpu_idle;
	idle_task->reg.r7 = idle_task->reg.r6 = 0; // stack and frame pointers
	idle_task->reg.psw = 0; // Disables interrupts PSW<1> = 0
}

void init_task1(void)
{
	struct task_struct *ts = (struct task_struct *)list_pop_front(&freequeue);

	ts->pid = 0;

	set_user_pages(ts);

}

void init_sched()
{
	int i;

	for (i = 0; i < NUM_TASKS; i++) {
		task[i].pid = -1;
		memset(&(&task[i])->regs, 0, sizeof(task[i].regs));
		memset(&(&task[i])->map, 0, sizeof(task[i].map));
	}

	init_queues();

	last_pid = 1; // Skip idle and task1 process
}

int kernel_main()
{
	tlb_setup_for_kernel();
	mm_init();

	init_sched();
	init_idle();
	init_task1();

	void (*user_entry)() = (void (*)())0x1000;

	user_entry();

	return 0;
}
