#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    /* Event distribution */
    switch (c->mcause) {
      case 0:  ev.event = EVENT_NULL; break;
      case 1:/* sys_yield */
      case 4:/* sys_write */
      case 9:/* sys_brk */ 
          ev.event = EVENT_SYSCALL; break;
      case -1: ev.event = EVENT_YIELD; break;
      default:
        printf("c->mcase=%d\n", c->mcause);
        ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  /* Registration__ am_ irq_ Handle callback function in __am_asm_trap */
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
