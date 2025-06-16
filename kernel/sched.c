#include "inc/sched.h"
#include "../sys/inc/logger.h"
#include "inc/mmu.h"
#include "inc/tasks.h"
#include "inc/uart.h"
#include <stddef.h>

#define USR_MODE 0b10000
#define CLR_MODE 0b11111

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
    tasks[task_index].sp = task_sp[task_index];
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
    // Save a cpsr with the USR mode activated,
    // so that the task1 and task2 run in usr mode.
    if (task_index != 0) {
      cpsr &= ~CLR_MODE;
      cpsr |= USR_MODE;
    }
    *tasks[task_index].irq_sp = cpsr;
    tasks[task_index].irq_sp -= 1;
    *tasks[task_index].irq_sp = save_sp;

    c_puts("The IRQ_SP would be: ");
    c_puts_hex((uint32_t)task_irq_sp[task_index]);
    c_putchar('\n');
    c_puts("The SVC_SP would be: ");
    c_puts_hex((uint32_t)task_sp[task_index]);
    c_putchar('\n');

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

  // Set SVC stack pointer to task0's stack before starting it for the first
  // time It's not strictly necessary, but it would be using the __svc_sp
  // (Kernel's sp) instead. Which can cause problems
  asm volatile("cps #0x13\n\t" // Switch to SVC mode
               "mov sp, %0\n\t"
               "cps #0x12\n\t" // Switch back to IRQ mode
               :
               : "r"(tasks[0].sp));

  // The IRQ is enabled on low, thats why bic instr is used
  // mrs r0, cpsr
  // bic r0, r0, 0x80
  // msr cpsr, r0
  // This instr replaces the 3instrs above
  // Enable interrupts
  asm volatile("cpsie i");
  current_task->entrypoint();
}
static inline uint32_t read_sp_usr(void);
static inline void write_sp_usr(uint32_t val);
static inline uint32_t read_sp_svc(void);
static inline void write_sp_svc(uint32_t val);
__attribute__((section(".kernel.text"))) uint32_t c_scheduler(_ctx_t *ctx) {
  current_task->current_ticks++;
  if (current_task->current_ticks >= current_task->task_ticks) {
    if (current_task->id == 0) {
      current_task->sp = (uint32_t *)read_sp_svc();
    } else {
      current_task->sp = (uint32_t *)read_sp_usr();
    }

    current_task->current_ticks = 0u;
    uint8_t id = current_task->id;
    id = (id + 1) % MAX_TASKS;
    c_log_taskswitch(id);
    current_task = &tasks[id];

    if (current_task->id == 0) {
      write_sp_svc((uint32_t)current_task->sp);
    } else {
      write_sp_usr((uint32_t)current_task->sp);
    }

    // Remove, used for debugging purposes
    //  for (int i = 0; i < 16; i++) {
    //    if (i == 15) {
    //      c_puts("The PC would be: ");
    //      c_puts_hex(current_task->irq_sp[i]);
    //      c_putchar('\n');
    //      c_puts("The SP would be: ");
    //      c_puts_hex((uint32_t)current_task->svc_sp);
    //      c_putchar('\n');
    //    } else {
    //      c_puts_hex(current_task->irq_sp[i]);
    //      c_putchar('\n');
    //    }
    //  }

    // Set the TTBR0 of the current_task
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(current_task->ttbr0));
  }
  return (uint32_t)current_task->irq_sp;
}

// https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/System-Instructions/Encoding-and-use-of-Banked-register-transfer-instructions/Register-arguments-in-the-Banked-register-transfer-instructions?lang=en
// The SYSTEM mode uses the same sp register as USER mode
static inline uint32_t read_sp_usr(void) {
  uint32_t val;
  asm volatile("mrs r1, cpsr\n\t"   // save current CPSR
               "cps #0x1F\n\t"      // switch to SYSTEM mode (usr regs)
               "mov %0, sp\n\t"     // read sp_usr
               "msr cpsr_c, r1\n\t" // restore CPSR
               : "=r"(val)::"r1");
  return val;
}

static inline void write_sp_usr(uint32_t val) {
  asm volatile("mrs r1, cpsr\n\t"
               "cps #0x1F\n\t"
               "mov sp, %0\n\t"
               "msr cpsr_c, r1\n\t" ::"r"(val)
               : "r1");
}

static inline uint32_t read_sp_svc(void) {
  uint32_t val;
  asm volatile("mrs r1, cpsr\n\t"   // save current CPSR
               "cps #0x13\n\t"      // switch to SVC mode
               "mov %0, sp\n\t"     // read sp
               "msr cpsr_c, r1\n\t" // restore CPSR
               : "=r"(val)::"r1");
  return val;
}

static inline void write_sp_svc(uint32_t val) {
  asm volatile("mrs r1, cpsr\n\t"
               "cps #0x13\n\t"
               "mov sp, %0\n\t"
               "msr cpsr_c, r1\n\t" ::"r"(val)
               : "r1");
}
