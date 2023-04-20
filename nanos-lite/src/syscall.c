#include <common.h>
#include <unistd.h>
#include "syscall.h"

void strace(uintptr_t index) {
    printf("  strace: %s\n", syscall_names[index]);
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  intptr_t ret;/* syscall ret number */

  switch (a[0]) {
    case SYS_exit: printf("++ exited with 0 ++\n"); ret = sys_exit(); break;
    case SYS_yield: ret = sys_yield(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  /* return system call status to user space */
  strace(a[0]); 
  c->GPR0 = ret;
}

int sys_yield() {
  yield();
  return 0;
}

int sys_exit() {
  halt(0);
  return 0;
}