#include <common.h>
#include <proc.h>

static Context* do_event(Event e, Context* c) {
  /* event processing */
  switch (e.event) {
    case EVENT_YIELD:
      // printf("\33[0mirq event yield!\n");
      c =  schedule(c);
      break;
    case EVENT_SYSCALL:
      // Log("EVENT_SYSCALL");
    case EVENT_NULL:
      // Log("EVENT_NULL");
      /* for self-trap exception(yield), which is used for time-sharing system
          mepc should be dnpc.
         for fault-type exception(e.g. page fault, memory fault)
          The returned PC and the PC that triggered the exception are the same.
       */
      // c->mepc += 4;//RISC did this by software
      do_syscall(c);break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  c->mepc += 4;//RISC did this by software

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
