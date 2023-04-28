#include <common.h>
// #include <unistd.h>
#include "syscall.h"

void strace(uintptr_t index) {
    printf("  strace: %s\n", syscall_names[index]);
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR0; // a0
  a[1] = c->GPR2; // a1
  a[2] = c->GPR3; // a2
  a[3] = c->GPR1; // a7 store the return value
  intptr_t ret;/* syscall ret number */

  switch (a[3]) {
    case SYS_exit: printf("++ exited with 0 ++\n"); ret = sys_exit(); break;
    case SYS_yield: ret = sys_yield(); break;
    /*
     * if don't use ret = , it will only print the first word, e.g. `hello` only print `h`
     *    why?
     *      the whole syscall process:
     *          navy:ecall -> am:__am_irq_handle -> nanos:do_event     -> nanos:do_syscall
     *                     -> irq.c:do_event     -> am:__am_irq_handle -> navy
     *      when add ret= ,the return exit code isn't 0 in navy, then write from printf
     *        decomposition will continue; without `ret =`, return register a0 will be 0,
     *        printf will exit.
     *
     */
    case SYS_write: ret = sys_write(a[0], (void*)a[1], a[2]); break;
    case SYS_brk: ret = sys_brk((void *)a[1]); break;
    case SYS_open: ret = fs_open((char *)a[0], a[1], a[2]); break;
    case SYS_read: ret = fs_read(a[0], (void *)a[1], a[2]); break;
    case SYS_lseek: ret = fs_lseek(a[0], a[1], a[2]); break;
    case SYS_close: ret = fs_close(a[0]); break;
    default: panic("Unhandled syscall ID = %d", a[3]);
  }
  // strace(a[3]); /* strace switch */
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


int sys_brk(void *addr) {
  /* for single task operating systems, 
   *  we can always return 0(true) now
   */
  return 0;
}