#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;/* context pointer( save the address of context ) */
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;
void naive_uload(PCB *pcb, const char *filename);

extern PCB *current;
Context* schedule(Context *prev);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);

#endif
