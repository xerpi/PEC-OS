#include "kernel.h"
#include "asmutils.h"

extern void (*cpu_idle)(void);

struct task_struct *current;
static struct task_struct *idle_task;

static struct task_struct task[NUM_TASKS];

static uint8_t phys_mem[NUM_FREE_PAGES];
static int num_free_frames;

static struct list_head freequeue;
static struct list_head readyqueue;

static uint8_t last_pid;
static uint8_t quantum_rr;
static uint16_t sisa_ticks;

static inline struct task_struct *list_pop_front_task_struct(struct list_head *l)
{
	return list_entry(list_pop_front(l), struct task_struct, list);
}
void timer_routine(void)
{
	sisa_ticks++;
	sched_schedule();
}

void key_routine(void)
{

}

void switch_routine(void)
{

}

void kb_routine(void)
{

}

syscall_value_t sys_fork()
{
	int i;
	int frame;
	struct task_struct *new;

	if (num_free_frames < NUM_USER_CODE_PAGES)
		return -1;

	if (list_empty(&freequeue))
		return -1;

	new = list_pop_front_task_struct(&freequeue);

	/* Copy current task_struct to the new one */
	//memcpy(new, current, sizeof(struct task_struct));
	for (i = 0; i < 10; i++) {
		new->regs[i] = current->regs[i];
	}
	for (i = 0; i < ARRAY_SIZE(current->map); i++) {
		new->map[i] = current->map[i];
	}

	/* Allocate new data pages and copy current's to them */
	/*for (i = 0; i < ARRAY_SIZE(task->map); i++) {
		if (task->map[i].type == PAGE_TYPE_DATA) {
			frame = mm_alloc_frame();
			task->map[i].pfn = frame;
			memcpy();
		}
	}*/

	new->reg.r1 = 0;
	new->pid = sched_get_free_pid();

	/* Place new to the readyqueue */
	list_add_tail(&new->list, &readyqueue);

	return new->pid;
}

syscall_value_t kill(syscall_value_t pid)
{
	return pid;
}

void mm_init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(phys_mem); i++) {
		phys_mem[i] = FRAME_FREE;
	}
}

int mm_alloc_frame(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(phys_mem); i++) {
		if (phys_mem[i] == FRAME_FREE) {
			phys_mem[i] = FRAME_USED;
			num_free_frames--;
			return i;
		}
	}
	return -1;
}

void mm_free_frame(uint8_t frame)
{
	phys_mem[frame] = FRAME_FREE;
	num_free_frames++;
}

void mm_frame_set_used(uint8_t frame)
{
	phys_mem[frame] = FRAME_USED;
	num_free_frames--;
}

void tlb_setup_for_kernel(void)
{
	int i;
	int j = 0;

	/* Kernel code */
	for (i = 0; i < NUM_KERNEL_CODE_PAGES; i++, j++) {
		/* ITLB */
		wrpi(i, (KERNEL_CODE_PAGE_START + i) | TLB_ENTRY_BIT_R |
			TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvi(i, KERNEL_CODE_PAGE_START + i);

		/* DTLB code read-only */
		wrpd(j, (KERNEL_CODE_PAGE_START + j) | TLB_ENTRY_BIT_R |
			TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvd(j, KERNEL_CODE_PAGE_START + j);
	}

	/* Kernel data (and stack) */
	for (i = 0; i < NUM_KERNEL_DATA_PAGES; i++, j++) {
		wrpd(j, (KERNEL_DATA_PAGE_START + i) | TLB_ENTRY_BIT_V | TLB_ENTRY_BIT_P);
		wrvd(j, KERNEL_DATA_PAGE_START + i);
		/* Mark the physical frame as used */
		mm_frame_set_used(KERNEL_DATA_PAGE_START + i);
	}
}

void tlb_setup_for_task(const struct task_struct *task)
{
	int i, j;
	uint8_t pc_entry;
	uint8_t num_mapped_itlb;
	uint8_t num_mapped_dtlb;

	/* From the entry 0 to NUM_KERNEL_PAGES we have the kernel */

	/* Find the page where the PC points to */
	for (i = 0; i < ARRAY_SIZE(task->map); i++) {
		if ((task->reg.pc >> PAGE_SHIFT) == task->map[i].vpn &&
		    task->map[i].v == 1 && task->map[i].type == PAGE_TYPE_CODE) {
			pc_entry = i;
			break;
		}
	}

	/* Map the page where the PC points to (just after the kernel code map) */
	wrpi(NUM_KERNEL_CODE_PAGES, task->map[pc_entry].pfn | (task->map[pc_entry].r << 4)
		| (task->map[pc_entry].v << 5) | (task->map[pc_entry].p << 6));
	wrvi(NUM_KERNEL_CODE_PAGES, task->map[pc_entry].vpn);

	/* Map the PC page to the DTLB  */
	wrpd(NUM_KERNEL_PAGES, task->map[pc_entry].pfn | (task->map[pc_entry].r << 4)
		| (task->map[pc_entry].v << 5) | (task->map[pc_entry].p << 6));
	wrvd(NUM_KERNEL_PAGES, task->map[pc_entry].vpn);

	num_mapped_itlb = NUM_KERNEL_CODE_PAGES + 1;
	num_mapped_dtlb = NUM_KERNEL_PAGES + 1;

	/* Map as most remaining code pages as we can */
	for (i = 0; i < ARRAY_SIZE(task->map); i++) {
		if (task->map[i].type != PAGE_TYPE_UNUSED && i != pc_entry) {
			if (task->map[i].type == PAGE_TYPE_CODE &&
			    num_mapped_itlb < TLB_NUM_ENTRIES) {
				/* Map this code page to both the I and DTLBs */
				wrpi(num_mapped_itlb,
					task->map[i].pfn | (task->map[i].r << 4) |
					(task->map[i].v << 5) | (task->map[i].p << 6));
				wrvi(num_mapped_itlb, task->map[i].vpn);

				wrpd(num_mapped_dtlb,
					task->map[i].pfn | (task->map[i].r << 4) |
					(task->map[i].v << 5) | (task->map[i].p << 6));
				wrvd(num_mapped_dtlb, task->map[i].vpn);

				num_mapped_itlb++;
				num_mapped_dtlb++;
			} else if (task->map[i].type == PAGE_TYPE_DATA &&
			           num_mapped_dtlb < TLB_NUM_ENTRIES) {
				wrpd(num_mapped_dtlb,
					task->map[i].pfn | (task->map[i].r << 4) |
					(task->map[i].v << 5) | (task->map[i].p << 6));

				wrvd(num_mapped_dtlb, task->map[i].vpn);

				num_mapped_dtlb++;
			}
		}
	}

	/* Try to map the VGA region to the DTLB */
	for (j = 0, i = num_mapped_dtlb; j < 2 && i < TLB_NUM_ENTRIES; i++, j++) {
		wrpd(i, (VGA_PAGE_START + j) | TLB_ENTRY_BIT_V);
		wrvd(i, VGA_PAGE_START + j);
	}

	num_mapped_dtlb += j;

	/* Empty the rest of the ITLB */
	for (i = num_mapped_itlb; i < TLB_NUM_ENTRIES; i++) {
		wrpi(i, 0);
		wrvi(i, 0);
	}

	/* Empty the rest of the DTLB */
	for (i = num_mapped_dtlb; i < TLB_NUM_ENTRIES; i++) {
		wrpd(i, 0);
		wrvd(i, 0);
	}
}


static int sched_need_switch(void)
{
	if (current->pid == 0) {
		// If executing idle_task and there are other tasks waiting
		// switch to them, else do nothing
		if (!list_empty(&readyqueue))
			return 1;
		else
			return 0;
	} else {
		// If not executing idle_task, switch only if quantum expired
		// Does not mind if only there is one user task
		quantum_rr--;
		if (quantum_rr == 0)
			return 1;
		else
			return 0;
	}
}

static void task_switch(struct task_struct *next)
{
	current = next;
	tlb_setup_for_task(current);
}

static void sched_next(struct task_struct *next)
{
	// Set quantum specified by the next task
	quantum_rr = next->quantum;
	task_switch(next);
}

void sched_schedule(void)
{
	struct task_struct *next;

	// Update task sched attrib. and check if switch needed
	if (sched_need_switch()) {
		if (current->pid != 0) {
			// Save current in readyqueue
			list_add_tail(&current->list, &readyqueue);
			next = list_pop_front_task_struct(&readyqueue);
		} else {
			if (list_empty(&readyqueue))
				next = idle_task;
			else
				next = list_pop_front_task_struct(&readyqueue);
		}
		/* Task switch to next */
		sched_next(next);
	}
}

uint8_t sched_get_free_pid(void)
{
	return ++last_pid;
}

void sched_init_queues(void)
{
	int i;

	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);

	for (i = 0; i < NUM_TASKS; i++)
		list_add_tail(&(&task[i])->list, &freequeue);
}

void sched_init_idle(void)
{
	int i;

	idle_task = list_pop_front_task_struct(&freequeue);

	/* Empty all the user TLB of the entries */
	for (i = 0; i < ARRAY_SIZE(task->map); i++)
		idle_task->map[i].type = PAGE_TYPE_UNUSED;

	idle_task->pid = 0;
	// Idle task does not have quantum since we task_switch to
	// any another user task if exists. Anyway we set up to default
	idle_task->quantum = SCHED_DEFAULT_QUANTUM;
	idle_task->reg.pc = (uintptr_t)&cpu_idle;
	/* Interrupts enabled, kernel mode */
	idle_task->reg.psw = (1 << 1) | 1;
}

void sched_init_task1(void)
{
	int i;
	int j = 0;
	struct task_struct *task1 = list_pop_front_task_struct(&freequeue);

	/* Setup code pages */
	for (i = 0; i < NUM_USER_CODE_PAGES; i++, j++) {
		task1->map[j].type = PAGE_TYPE_CODE;
		task1->map[j].vpn = USER_PAGE_START + j;
		task1->map[j].pfn = USER_PAGE_START + j;
		task1->map[j].r = 1;
		task1->map[j].v = 1;
		task1->map[j].p = 0;
		/* Mark the physical frame as used */
		mm_frame_set_used(USER_PAGE_START + j);
	}

	/* Setup data pages */
	for (i = 0; i < NUM_USER_DATA_PAGES; i++, j++) {
		task1->map[j].type = PAGE_TYPE_DATA;
		task1->map[j].vpn = USER_PAGE_START + j;
		task1->map[j].pfn = USER_PAGE_START + j;
		task1->map[j].r = 0;
		task1->map[j].v = 1;
		task1->map[j].p = 0;
		/* Mark the physical frame as used */
		mm_frame_set_used(USER_PAGE_START + j);
	}

	/* Empty the rest of the entries */
	for (; j < ARRAY_SIZE(task->map); j++)
		task1->map[j].type = PAGE_TYPE_UNUSED;

	task1->pid = 1;
	task1->quantum = SCHED_DEFAULT_QUANTUM;
	task1->reg.pc = USER_PAGE_START << PAGE_SHIFT;
	/* Interrups enabled, user mode */
	task1->reg.psw = (1 << 1);

	/* Sched starts with quantum from task1 */
	quantum_rr = task1->quantum;

	tlb_setup_for_task(task1);

	current = task1;
}

void sched_init(void)
{
	int i;

	for (i = 0; i < NUM_TASKS; i++) {
		task[i].pid = -1;
		memset(&(&task[i])->regs, 0, sizeof(task[i].regs));
		memset(&(&task[i])->map, 0, sizeof(task[i].map));
	}

	sched_init_queues();
	sched_init_idle();
	sched_init_task1();

	/* Skip idle and task1 processes */
	last_pid = 1;
}

int kernel_main(void)
{
	mm_init();
	tlb_setup_for_kernel();
	sched_init();

	void (*user_entry)(void) = (void (*)(void))(USER_PAGE_START << PAGE_SHIFT);

	/* Enable interrupts, user mode and jump to the user code */
	__asm__(
		"wrs s0, %0\n\t"
		"wrs s1, %1\n\t"
		"reti"
		: : "r"(PSW_USER_MODE | PSW_IE), "r"(user_entry)
	);

	return 0;
}
