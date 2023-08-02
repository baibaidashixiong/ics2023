#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
const char* file_name = "/bin/dummy";
void naive_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, NULL);/* create hello_fun as kernel thread */
  printf("new pcb pc is %p\n", &pcb[0]);
  switch_boot_pcb();

  Log("Initializing processes...");

  naive_uload(NULL, file_name);
  // load program here

}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

  // always select pcb[0] as the new process now
  current = &pcb[0];
  // printf("new process pc is %p\n", current->cp);

  // then return the new context
  return current->cp;
}
