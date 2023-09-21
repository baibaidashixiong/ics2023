#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
const char* file_name = "/bin/pal";
void naive_uload(PCB *pcb, const char *filename);

char *argv_pal[] = {"/bin/pal", "--skip", NULL};
void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, "zqz");/* create hello_fun as kernel thread */
  char *argv1[] = {"/bin/nterm", NULL};
  context_uload(&pcb[1], "/bin/nterm", argv1, NULL);
  // char *argv_exec_test[] = {"/bin/exec-test", NULL};
  // context_uload(&pcb[1], "/bin/hello", NULL, NULL);
  // context_uload(&pcb[1], "/bin/exec-test", argv_exec_test, NULL);
  // context_uload(&pcb[2], "/bin/pal", NULL, NULL);

  switch_boot_pcb();/* set pcb stub */

  Log("Initializing processes...");

  // naive_uload(NULL, file_name);
  // load program here

}
// int k = 0;
Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

  // always select pcb[0] as the new process now
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // printf("new process pc is %p\n", current->cp);

  // then return the new context
  // printf("the %d th time current cp is %p\n", k++, current->cp);
  return current->cp;
}
