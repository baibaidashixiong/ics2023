/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

const char *p_regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  /* 
   * difftest: check with right register and pc for every instruction
   *  for riscv, the absolutely correct implementation tool is Spike
   */
  int reg_num = ARRLEN(cpu.gpr);
  for (int i = 0; i < reg_num; i++) {
    if (ref_r->gpr[i] != cpu.gpr[i]) {
      //printf("\33[1;31rigster[%d] gpr wrong!right gpr is 0x%x, wrong gpr is 0x%x\n\33[0m", i, ref_r->gpr[i] , cpu.gpr[i]);
      return false;
    }
  }
  if (ref_r->pc != cpu.pc) {
    printf("\33[1;31mpc wrong!right pc is 0x%x, wrong pc is 0x%x\n\33[0m",ref_r->pc, cpu.pc);
    return false;
  }
  if(ref_r->csr.mstatus != cpu.csr.mstatus) {
    printf("mstatus wrong!right mstatus is 0x%x, wrong status is 0x%x\n",ref_r->csr.mstatus, cpu.csr.mstatus);
    return false;
  }
  if(ref_r->csr.mtvec != cpu.csr.mtvec) {
    panic("mtvec wrong!right mtvec is 0x%x, wrong mtvec is 0x%x\n",ref_r->csr.mtvec, cpu.csr.mtvec);
    return false;
  }
  if(ref_r->csr.mcause != cpu.csr.mcause) {
    for(int i=0;i<32;i++){
      printf("%-3s: 0x%08x  |  ",p_regs[i],cpu.gpr[i]);
    if( (i+1) % 8 == 0) printf("\n");
    }
    printf("pc : %x\n", cpu.pc);
    printf("mstatus : 0x%08x\n",cpu.csr.mstatus);
    printf("mtvec : 0x%08x\n",cpu.csr.mtvec);
    printf("mcause : 0x%08x\n",cpu.csr.mcause);
    printf("mepc : 0x%08x\n",cpu.csr.mepc);
    panic("mcause wrong!right mcause is 0x%x, wrong mcause is 0x%x\n",ref_r->csr.mcause, cpu.csr.mcause);
    return false;
  }
  if(ref_r->csr.mepc != cpu.csr.mepc) {
    panic("mepc wrong!right mepc is 0x%x, wrong mepc is 0x%x\n",ref_r->csr.mepc, cpu.csr.mepc);
    return false;
  }
  return true;
}

void isa_difftest_attach() {
}
