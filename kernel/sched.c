#include "inc/sched.h"
#include "inc/mmu.h"
#include "inc/tasks.h"
#include "inc/uart.h"
#include <stddef.h>

static volatile _systick_t systick = 0;

__attribute__((section(".text"))) void c_systick_handler() { systick++; }

__attribute__((section(".text"))) _systick_t c_systick_get() { return systick; }

__attribute__((section(".text"))) void c_delay(_systick_t ticks) {
  _systick_t start = systick;
  while (systick - start < ticks) {
  }
}

static _task_t tasks[MAX_TASKS];
static uint8_t task_index = 0;
static _task_t *current_task = NULL;

/* Stack pointers for tasks */
extern uint32_t __task0_sp;
extern uint32_t __task1_sp;
extern uint32_t __task2_sp;
extern uint32_t __task3_sp;
uint32_t *task_sp[] = {&__task0_sp, &__task1_sp, &__task2_sp, &__task3_sp};

/* IRQ stack pointers for tasks */
extern uint32_t __task0_irq_sp;
extern uint32_t __task1_irq_sp;
extern uint32_t __task2_irq_sp;
extern uint32_t __task3_irq_sp;
uint32_t *task_irq_sp[] = {&__task0_irq_sp, &__task1_irq_sp, &__task2_irq_sp,
                           &__task3_irq_sp};

/* MMU */
extern uint32_t *g_next_l2_addr;
extern uint32_t g_kernel_l1_table[];

__attribute__((section(".text"))) void c_task_init(_task_ptr_t entrypoint,
                                                   _systick_t ticks) {

  // Save the cpsr with the IRQ bit set, so it can be pushed to the stack
  uint32_t cpsr;
  asm volatile("mrs %0, cpsr" : "=r"(cpsr));
  asm volatile("bic %0, %0, #0x80" : "+r"(cpsr));

  if (task_index < MAX_TASKS) {
    tasks[task_index].id = task_index;
    tasks[task_index].entrypoint = entrypoint;
    tasks[task_index].task_ticks = ticks;
    tasks[task_index].current_ticks = 0u;
    /* Set stack pointer for task */
    tasks[task_index].svc_sp = task_sp[task_index];
    /* Set IRQ stack pointer for task */
    tasks[task_index].irq_sp = task_irq_sp[task_index];

    // Set Up the lr to point to the task's entrypoint
    *tasks[task_index].irq_sp = (uint32_t)entrypoint;
    // Push the registers and cpsr to the stack
    for (int i = 0; i < 13; i++) {
      tasks[task_index].irq_sp -= 1;
      *tasks[task_index].irq_sp = 0;
    }

    uint32_t save_sp = (uint32_t)tasks[task_index].irq_sp;
    tasks[task_index].irq_sp -= 1;
    *tasks[task_index].irq_sp = cpsr;
    tasks[task_index].irq_sp -= 1;
    *tasks[task_index].irq_sp = save_sp;

    // Set the TTBR0 address for each task
    uint32_t *ttbr0;
    if (task_index == 0) {
      // Means it's the kernel/idle task
      ttbr0 = g_kernel_l1_table;
    } else {
      // The g_next_l2_addr will be pointing to the end
      // of the following structure:
      // L1 16KB
      // 4*L2 1KB
      // So, we set the TTBR0 of the task to the new L1_TABLE
      // And then we init the MMU with it
      ttbr0 = g_next_l2_addr;
    }
    tasks[task_index].ttbr0 = ttbr0;
    // TODO, the -1024 is to match the length of the stack
    // the arrays store the sp_end position, we need the sp_start
    // Remember that the stack starts from a low memory position
    // and goes to a higher memory addr.
    c_mmu_fill_tables(ttbr0, (uint32_t)task_sp[task_index] - 1024,
                      (uint32_t)task_irq_sp[task_index] - 1024);

    task_index++;
  }
}

__attribute__((section(".text"))) void c_scheduler_init(void) {
  c_task_init(task_idle, 5u);
  c_task_init(task1, 8u);
  c_task_init(task2, 12u);
  c_task_init(task3, 5u);
  current_task = &tasks[0];

  // Set the TTBR0 register
  asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(current_task->ttbr0));
  // Start the MMU
  c_mmu_init();

  c_putsln("Board init done");

  // The IRQ is enabled on low, thats why bic instr is used
  // mrs r0, cpsr
  // bic r0, r0, 0x80
  // msr cpsr, r0
  // This instr replaces the 3instrs above
  // Enable interrupts
  asm volatile("cpsie i");
  current_task->entrypoint();
}

__attribute__((section(".text"))) uint32_t c_scheduler(_ctx_t *ctx) {
  current_task->current_ticks++;
  if (current_task->current_ticks >= current_task->task_ticks) {
    // Switch to SVC mode to save svc_sp
    uint32_t svc_sp;
    asm volatile("cps #0x13    \n\t" // Switch to SVC mode
                 "mov %0, sp   \n\t" // Save the SVC stack pointer
                 : "=r"(svc_sp));
    current_task->svc_sp = (uint32_t *)svc_sp;
    // Switch back to IRQ mode
    asm volatile("cps #0x12");

    // Set the current_ticks to 0
    current_task->current_ticks = 0u;
    // Increment the task_id by 1 and wrap around
    uint8_t id = current_task->id;
    id++;
    id %= MAX_TASKS;
    // Set the current task to the new task_id
    current_task = &tasks[id];

    // Switch to SVC mode to use the new task's svc_sp
    asm volatile("cps #0x13    \n\t" // Switch to SVC mode
                 "mov sp, %0   \n\t" // Load the new task's SVC stack pointer
                 : "=r"(current_task->svc_sp));
    // Switch back to IRQ mode
    asm volatile("cps #0x12");
    // Set the TTBR0 of the current_task
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(current_task->ttbr0));

    c_puts_hex(current_task->id);
    c_putchar('\n');
  }
  return (uint32_t)current_task->irq_sp;
}
