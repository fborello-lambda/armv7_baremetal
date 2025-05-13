#ifndef __SCHED_H__
#define __SCHED_H__

#include <stdint.h>

typedef uint32_t _systick_t;
typedef struct {
  uint32_t *sp;
  uint32_t spsr;
  uint32_t registers[13];
  uint32_t *lr;
} _ctx_t;

// Function Definitions
void c_systick_handler();
_systick_t c_systick_get();
void c_delay(_systick_t ticks);

// Task Definitions

typedef void (*_task_ptr_t)(void);
typedef uint8_t _task_id_t;

typedef struct {
  uint32_t *svc_sp;
  uint32_t *irq_sp;
  uint32_t *ttbr0;
  _task_id_t id;
  _task_ptr_t entrypoint;
  _systick_t task_ticks;
  _systick_t current_ticks;
} _task_t;

#define MAX_TASKS 4u

// Function Definitions

void c_task_init(_task_ptr_t entrypoint, _systick_t ticks);
void c_scheduler_init(void);
uint32_t c_scheduler(_ctx_t *);
#endif
