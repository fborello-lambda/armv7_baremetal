#include "inc/sched.h"
#include "../sys/inc/logger.h"
#include "inc/mmu.h"
#include "inc/tasks.h"
#include "inc/uart.h"
#include <stddef.h>

static volatile _systick_t systick = 0;

__attribute__((section(".kernel.text"))) void c_systick_handler() { systick++; }

__attribute__((section(".kernel.text"))) _systick_t c_systick_get() {
  return systick;
}

__attribute__((section(".kernel.text"))) void c_delay(_systick_t ticks) {
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
uint32_t *task_sp[] = {&__task0_sp, &__task1_sp, &__task2_sp};

/* IRQ stack pointers for tasks */
extern uint32_t __task0_irq_sp;
extern uint32_t __task1_irq_sp;
extern uint32_t __task2_irq_sp;
uint32_t *task_irq_sp[] = {&__task0_irq_sp, &__task1_irq_sp, &__task2_irq_sp};

/* MMU */
// TODO: i should isolate the tables inside each task's .data section maybe.
mmu_tables_t mmu_tables[MAX_TASKS] __attribute__((section(".mmu_tables")));

__attribute__((section(".kernel.text"))) void
c_task_init(_task_ptr_t entrypoint, _systick_t ticks) {

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
    uint32_t *ttbr0 = mmu_tables[task_index].l1_table;
    tasks[task_index].ttbr0 = ttbr0;
    c_mmu_fill_tables(&mmu_tables[task_index], task_index);

    task_index++;
  }
}

__attribute__((section(".kernel.text"))) void c_scheduler_init(void) {
  c_task_init(task_idle, 10u);
  c_task_init(task1, 10u);
  c_task_init(task2, 10u);
  current_task = &tasks[0];

  // Set the TTBR0 register
  asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(current_task->ttbr0));
  // Start the MMU
  c_mmu_init();

  c_log_info("Scheduler init Done");

  // The IRQ is enabled on low, thats why bic instr is used
  // mrs r0, cpsr
  // bic r0, r0, 0x80
  // msr cpsr, r0
  // This instr replaces the 3instrs above
  // Enable interrupts
  asm volatile("cpsie i");
  current_task->entrypoint();
}

__attribute__((section(".kernel.text"))) uint32_t c_scheduler(_ctx_t *ctx) {
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

    uint32_t *new_svc_sp = current_task->svc_sp;
    asm volatile("cps #0x13    \n\t" // Switch to SVC mode
                 "mov sp, %0   \n\t" // Load the new task's SVC stack pointer
                 :
                 : "r"(new_svc_sp));
    // Switch back to IRQ mode
    asm volatile("cps #0x12");
    // Set the TTBR0 of the current_task
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(current_task->ttbr0));

    c_log_taskswitch(current_task->id);

    for (int i = 0; i < 16; i++) {
      if (i == 15) {
        c_puts("The PC would be: ");
        c_puts_hex(current_task->irq_sp[i]);
        c_putchar('\n');
      }
    }
  }
  return (uint32_t)current_task->irq_sp;
}
