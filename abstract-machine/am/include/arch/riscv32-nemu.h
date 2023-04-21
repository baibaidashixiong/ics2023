#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define GPR0 gpr[10] // a0
#define GPR1 gpr[17] // a7
#define GPR2 gpr[11] // a1
#define GPR3 gpr[12] // a2
#define GPR4 gpr[0]

#endif
