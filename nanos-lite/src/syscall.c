#include <common.h>
#include <unistd.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  intptr_t ret;/* syscall ret number */

  switch (a[0]) {
    case SYS_exit: ret = sys_exit(); break;
    case SYS_yield: ret = sys_yield(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  /* return system call status to user space */
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