#include <common.h>

static Context* do_event(Event e, Context* c) {
  /* event processing */
  switch (e.event) {
    case EVENT_YIELD: printf("\33[0mirq event yield!\n");break;
    case EVENT_SYSCALL:
      // Log("EVENT_SYSCALL");
    case EVENT_NULL:
      // Log("EVENT_NULL");
      c->mepc += 4;
      do_syscall(c);break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
