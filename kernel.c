#include "kernel.h"
#include "asmutils.h"

static struct task_struct task[NUM_TASKS];

static uint8_t phys_mem[NUM_FREE_PAGES];

static struct list_head freequeue;
static struct list_head readyqueue;


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

void init_queues()
{
	int i;

	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);

	for (i = 0; i < NUM_TASKS; i++)
		list_add_tail(&task[i].list, &freequeue);
}

int kernel_main()
{
	tlb_setup_for_kernel();
	mm_init();
	init_queues();

	void (*user_entry)() = (void (*)())0x1000;

	user_entry();

	return 0;
}
